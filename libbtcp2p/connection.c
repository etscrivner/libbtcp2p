#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>

#include <openssl/sha.h>

#include "libbtcp2p/connection.h"
#include "libbtcp2p/log.h"
#include "libbtcp2p/pack.h"
#include "libbtcp2p/types.h"
#include "libbtcp2p/vartypes.h"

static const char* BTCP2P_USER_AGENT = "/btcp2p:0.0.1/";

static const struct btcp2p_chain_t CHAINS[] = {
  [0] = { .name = "mainnet", .version = 70015, .magic = 0xD9B4BEF9, .port = 8333 },
  [1] = { .name = "regtest", .version = 70015, .magic = 0xDAB5BFFA, .port = 18444 },
  [2] = { .name = "testnet", .version = 70015, .magic = 0x0709110B, .port = 18333 },
  [3] = { 0 }
};

static struct btcp2p_chain_t const *
btcp2p_chain_for_network(char const * const network) {
  for (struct btcp2p_chain_t const * next = CHAINS;
       next->name != NULL;
       next++)
  {
    if (strncmp(next->name, network, 7) == 0) return next;
  }

  return NULL;
}

static void btcp2p_netaddr_create(struct btcp2p_netaddr_t* addr,
                                  uint64_t services,
                                  char* ipv4_address,
                                  uint16_t port) {
  addr->services = services;
  for (int i = 0; i < 10; i++) {
    addr->address[i] = 0x00;
  }
  addr->address[10] = 0xFF;
  addr->address[11] = 0xFF;
  inet_pton(AF_INET, ipv4_address, &addr->address[12]);
  addr->port = htons(port);
}

// btcp2p_payload_checksum returns the first 4 bytes of the double-SHA256 hash
// of the given payload.
uint32_t btcp2p_payload_checksum(uint8_t const * const payload,
                                 size_t payload_size)
{
  static unsigned char m1[SHA256_DIGEST_LENGTH];
  static unsigned char m2[SHA256_DIGEST_LENGTH];
  SHA256(SHA256(payload, payload_size, m1), SHA256_DIGEST_LENGTH, m2);
  return *(uint32_t*)&m2;
}

// btcp2p_sendall blocks until all of the given data is successfully sent.
static int btcp2p_sendall(int socket, uint8_t* data, int datalen) {
  int total = 0;
  int bytesleft = datalen;
  int n = 0;

  while (total < datalen) {
    n = send(socket, data + total, bytesleft, 0);
    if (n == -1) {
      break;
    }
    total += n;
    bytesleft -= n;
  }

  return n == -1 ? -1 : 0;
}

static bool btcp2p_recv_message(struct btcp2p_connection_t* connection,
                                struct btcp2p_message_t* message)
{
  ssize_t result = recv(
    connection->socket,
    &message->header,
    sizeof(message->header),
    MSG_WAITALL
  );

  if (result == 0) {
    btcp2p_log(BTCP2P_LOG_ERROR, "remote host closed connection.\n");
    return false;
  }
  if (result < 0) {
    btcp2p_log(BTCP2P_LOG_ERROR, "header recv error: %s\n", strerror(errno));
    return false;
  }

  if (message->header.length > 0) {
    // TODO: Move this stuff into the checked buffer abstraction
    if (message->payload.capacity < message->header.length) {
      btcp2p_checked_buffer_resize(&message->payload, message->header.length);
    }

    result = recv(
      connection->socket,
      message->payload.buffer,
      message->header.length,
      MSG_WAITALL
    );

    if (result == 0) {
      btcp2p_log(BTCP2P_LOG_ERROR, "remote host closed connection.\n");
      return false;
    }
    if (result < 0) {
      btcp2p_log(BTCP2P_LOG_ERROR, "payload recv error: %s\n", strerror(errno));
      return false;
    }

    message->payload.len = message->header.length;
    message->payload.rw_cursor = 0;
  }

  // Validate the checksum of the message
  uint32_t actual_checksum = btcp2p_payload_checksum(
    message->payload.buffer,
    message->header.length
  );
  if (message->header.checksum != actual_checksum) {
    btcp2p_log(
      BTCP2P_LOG_ERROR,
      "invalid message checksum: expected %08x was %08x.\n",
      message->header.checksum,
      actual_checksum
    );
    return false;
  }

  return true;
}

bool btcp2p_perform_handshake(struct btcp2p_connection_t* conn)
{
  struct btcp2p_varstr_t varstr = { { 0 }, 0 };
  btcp2p_varstr_encode(&varstr, (char*)BTCP2P_USER_AGENT, strlen(BTCP2P_USER_AGENT));

  btcp2p_pack_and_send_message(
    conn,
    "version",
    "ilLNNojIb",
    70015,
    0,
    time(NULL),
    conn->addr_recv,
    conn->addr_from,
    /* nonce, */
    varstr,
    0,
    true
  );

  if (!btcp2p_recv_message(conn, &conn->message)) {
    return false;
  }

  // TODO: Just make btcp2p_recv_message set this
  conn->has_message = true;
  if (!btcp2p_has_message(conn, "version")) {
    btcp2p_log(BTCP2P_LOG_ERROR, "did not receive version message\n");
    return false;
  }

  if (!btcp2p_recv_message(conn, &conn->message)) {
    return false;
  }

  conn->has_message = true;
  if (!btcp2p_has_message(conn, "verack")) {
    btcp2p_log(BTCP2P_LOG_ERROR, "did not receive verack message\n");
    return false;
  }

  if (!btcp2p_pack_and_send_message(conn, "verack", "")) {
    return false;
  }

  return true;
}

bool btcp2p_connect(struct btcp2p_connection_t* connection,
                    char const * const network,
                    char const * const ipv4_address)
{
  connection->chain = btcp2p_chain_for_network(network);
  if (!connection->chain) {
    btcp2p_log(
      BTCP2P_LOG_ERROR,
      "unable to find configuration for network '%s'\n",
      network
    );
    return false;
  }

  // Get string representation of the port (5 chars + 1 char for NULL
  // termination)
  char port_string[6];
  snprintf(port_string, 6, "%d", connection->chain->port);

  // Get address information for th remote host
  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  int status;
  if ((status = getaddrinfo(ipv4_address,
                            port_string,
                            &hints,
                            &connection->remote_address)) != 0)
  {
    btcp2p_log(
      BTCP2P_LOG_ERROR,
      "getaddrinfo failed: %s\n",
      gai_strerror(status)
    );
    return false;
  }

  // Open a new socket file descriptor
  connection->socket = socket(
    connection->remote_address->ai_family,
    connection->remote_address->ai_socktype,
    connection->remote_address->ai_protocol
  );
  if (connection->socket < 0) {
    btcp2p_log(
      BTCP2P_LOG_ERROR,
      "unable to open socket: %s\n",
      strerror(errno)
    );
    freeaddrinfo(connection->remote_address);
    
    return false;
  }

  // TODO: Use poll instead of select here for consistency.

  // Place the socket into non-blocking mode before attempting to connect in
  // order to set a connection timeout.
  int opts = fcntl(connection->socket, F_GETFL);
  fcntl(connection->socket, F_SETFL, opts | O_NONBLOCK);

  struct timeval tv;
  tv.tv_sec = 10;
  tv.tv_usec = 0;

  fd_set readfds;
  fd_set writefds;
  FD_ZERO(&readfds);
  FD_ZERO(&writefds);

  FD_SET(connection->socket, &readfds);
  FD_SET(connection->socket, &writefds);

  // Connect to the remote host.
  status = connect(
    connection->socket,
    connection->remote_address->ai_addr,
    connection->remote_address->ai_addrlen
  );

  // Error out if we aren't actually connecting
  if (status != -1 || errno != EINPROGRESS) {
    btcp2p_log(
      BTCP2P_LOG_ERROR,
      "unable to connect: %s\n",
      strerror(errno)
    );

    freeaddrinfo(connection->remote_address);
    close(connection->socket);

    return false;
  }

  // Wait until we either connect or timeout
  if (select(connection->socket+1, &readfds, &writefds, NULL, &tv) <= 0) {
    btcp2p_log(
      BTCP2P_LOG_ERROR,
      "connect timeout or error: %s.\n",
      strerror(errno)
    );

    freeaddrinfo(connection->remote_address);
    close(connection->socket);

    return false;
  }

  // Check what happened with our connection
  int error_val;
  socklen_t error_len = sizeof(error_val);
  if (getsockopt(connection->socket, SOL_SOCKET, SO_ERROR, &error_val, &error_len) < 0) {
    btcp2p_log(
      BTCP2P_LOG_ERROR,
      "getsockopt error: %s.\n",
      strerror(errno)
    );

    freeaddrinfo(connection->remote_address);
    close(connection->socket);

    return false;
  }

  if (error_val != 0) {
    btcp2p_log(
      BTCP2P_LOG_ERROR,
      "connection error: %s.\n",
      strerror(error_val)
    );

    freeaddrinfo(connection->remote_address);
    close(connection->socket);

    return false;
  }

  // Place the socket back into blocking mode
  fcntl(connection->socket, F_SETFL, opts);

  btcp2p_checked_buffer_create(&connection->message.payload);
  if (!btcp2p_perform_handshake(connection)) {
    btcp2p_checked_buffer_destroy(&connection->message.payload);
    freeaddrinfo(connection->remote_address);
    close(connection->socket);

    return false;
  }

  // Set the receiving address
  btcp2p_netaddr_create(&connection->addr_recv, ~0, "127.0.0.1", connection->chain->port);

  // Set the sending address
  connection->addr_from.services = ~0;
  if (connection->remote_address->ai_family == AF_INET6) {
    // TODO(eric): See if this actually works...
    struct sockaddr_in6* addr = ((struct sockaddr_in6*)connection->remote_address);
    memcpy(connection->addr_from.address, &addr->sin6_addr, 16);
    connection->addr_from.port = htons(addr->sin6_port);
  } else {
    // Write the twelve bytes:
    // 00 00 00 00 00 00 00 00 00 00 FF FF
    // Followed by 4 bytes of address data in network format.
    memset(connection->addr_from.address, 0, 10);
    connection->addr_from.address[10] = 0xFF;
    connection->addr_from.address[11] = 0xFF;

    struct sockaddr_in* addr = (struct sockaddr_in*)connection->remote_address->ai_addr;
    *((uint32_t*)&connection->addr_from.address[12]) = *(uint32_t*)&addr->sin_addr;
    connection->addr_from.port = htons(addr->sin_port);
  }

  return true;
}

void btcp2p_disconnect(struct btcp2p_connection_t* connection) {
  close(connection->socket);
  freeaddrinfo(connection->remote_address);
  btcp2p_checked_buffer_destroy(&connection->message.payload);
}

bool btcp2p_message_pump(struct btcp2p_connection_t* connection)
{
  connection->has_message = false;

  struct pollfd pfd;
  pfd.fd = connection->socket;
  pfd.events = POLLIN | POLLHUP | POLLRDNORM;
  pfd.revents = 0;

  if (poll(&pfd, 1, 100 /* milliseconds */) > 0) {
    if (!btcp2p_recv_message(connection, &connection->message)) {
      return false;
    }

    connection->has_message = true;
  }

  return true;
}

bool btcp2p_has_message(struct btcp2p_connection_t* connection,
                        char const command[12])
{
  if (!connection->has_message) {
    return false;
  }

  if (command != NULL) {
    if (strncmp(connection->message.header.command, command, 12) != 0) {
      return false;
    }
  }

  return true;
}

bool btcp2p_unpack_message(struct btcp2p_connection_t* connection,
                           char const * const format,
                           ...)
{
  if (!connection->has_message) {
    return false;
  }

  va_list args;
  va_start(args, format);
  btcp2p_vunpack(&connection->message.payload, format, args);
  va_end(args);

  return true;
}

bool btcp2p_pack_and_send_message(struct btcp2p_connection_t* connection,
                                  char const command[12],
                                  char const * const format,
                                  ...)
{
  // Set this to false since we're about to use the message
  connection->has_message = false;

  btcp2p_checked_buffer_prepare_write(&connection->message.payload);
  connection->message.header.magic = connection->chain->magic;
  strncpy(connection->message.header.command, command, 12);

  va_list args;
  va_start(args, format);
  connection->message.header.length = btcp2p_vpack(
    &connection->message.payload,
    format,
    args
  );
  va_end(args);

  connection->message.header.checksum = btcp2p_payload_checksum(
    connection->message.payload.buffer,
    connection->message.header.length
  );

  if (btcp2p_sendall(connection->socket,
                     (uint8_t*)&connection->message.header,
                     sizeof(connection->message.header)) < 0)
  {
    btcp2p_log(
      BTCP2P_LOG_ERROR,
      "message header send failed: %s\n",
      strerror(errno)
    );
    return false;
  }

  if (btcp2p_sendall(connection->socket,
                     connection->message.payload.buffer,
                     connection->message.header.length) < 0)
  {
    btcp2p_log(
      BTCP2P_LOG_ERROR,
      "message payload send failed: %s\n",
      strerror(errno)
    );
    return false;
  }

  return true;
}
