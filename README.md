# libbtcp2p

A simple C-interface for interacting with nodes speaking the [Bitcoin P2P protocol](https://en.bitcoin.it/wiki/Protocol_documentation).

* Documentation
* Examples

## Example

```c
#include <libbtcp2p/btcp2p.h>

int main() {
  struct btcp2p_connection_t connection = { 0 };
  
  struct btcp2p_timer_t ping_timer = { 0 };
  btcp2p_timer_init(&ping_timer, 10 /* secs */);
  
  if (btcp2p_connect(&connection, "testnet", "127.0.0.1")) {
    while (btcp2p_message_pump(&connection) && IsRunning) {
      if (btcp2p_has_message(&connection, "ping")) {
        uint64_t nonce;
        btcp2p_unpack_message(&connection, "l", &nonce);
        btcp2p_pack_and_send_message(&connection, "pong", "l", nonce);
      }

      if (btcp2p_timer_expired(&ping_timer)) {
        btcp2p_log(BTCP2P_LOG_INFO, "timer expired, sending ping.\n");
        btcp2p_pack_and_send_message(&connection, "ping", "o");
        btcp2p_timer_reset(&ping_timer);
      }

      /* Any message */
      if (btcp2p_has_message(&connection, NULL)) {
        btcp2p_log(BTCP2P_LOG_INFO, "got message %.*s\n", 12, connection.message.header.command);
      }
    }

    btcp2p_disconnect(&connection);
  }
  
  return 0;
}
```

## Building

```
make
```

## Build Example

```
make btcp2p_example
```

## Testing

```
make check
```
