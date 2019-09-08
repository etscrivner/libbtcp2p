#ifndef LIBBTCP2P_CONNECTION_H
#define LIBBTCP2P_CONNECTION_H

#include <stdbool.h>
#include <stdint.h>

#include "libbtcp2p/checked_buffer.h"
#include "libbtcp2p/types.h"

struct btcp2p_message_header_t {
  uint32_t magic;
  char command[12];
  uint32_t length;
  uint32_t checksum;
};

struct btcp2p_message_t {
  struct btcp2p_message_header_t header;
  struct btcp2p_checked_buffer_t payload;
};

struct btcp2p_chain_t {
  char const * const name;
  int32_t version;
  uint32_t magic;
  uint16_t port;
};

struct btcp2p_connection_t {
  int socket;
  struct addrinfo* remote_address;
  struct btcp2p_chain_t const * chain;
  bool has_message; ///< Did we receive a message on most recent poll?
  struct btcp2p_message_t message; ///< Last message received on network.
  struct btcp2p_netaddr_t addr_from;
  struct btcp2p_netaddr_t addr_recv;
};

// TODO: Add the ability to specify a port

// btcp2p_connect opens a new TCP socket connection to the given IP address
// serving the giving Bitcoin network.
bool btcp2p_connect(struct btcp2p_connection_t* connection,
                    char const * const network,
                    char const * const ipv4_address);

// btcp2p_disconnect closes an open connection and cleans up resources.
void btcp2p_disconnect(struct btcp2p_connection_t* connection);

// btcp2p_message_pump polls for new messages on the socket. Returns true
// unless there was an error receiving messages on the socket or the socket was
// closed by the remote host.
bool btcp2p_message_pump(struct btcp2p_connection_t* connection);

// btcp2p_has_message indicates whether or not a message for the given command
// has been received. If the command given is NULL then it is true if there was
// any message received.
bool btcp2p_has_message(struct btcp2p_connection_t* connection,
                        char const command[12]);

// btcp2p_unpack_message unpacks a message from the connection
bool btcp2p_unpack_message(struct btcp2p_connection_t* connection,
                           char const * const format,
                           ...);

bool btcp2p_pack_and_send_message(struct btcp2p_connection_t* connection,
                                  char const command[12],
                                  char const * const restrict format,
                                  ...);

#endif // LIBBTCP2P_CONNECTION_H
