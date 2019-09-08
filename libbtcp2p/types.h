#ifndef LIBBTCP2P_TYPES_H
#define LIBBTCP2P_TYPES_H

#include <stdint.h>

struct btcp2p_varint_t {
  uint8_t length; ///< The length of the data field.
  uint8_t data[9]; ///< Contains packed variable length integer data
  uint64_t value; ///< The actual value that was packed
};

struct btcp2p_varstr_t {
  struct btcp2p_varint_t length;
  char* data;
};

struct btcp2p_netaddr_t {
  uint32_t time;
  uint64_t services;
  char address[16];
  uint16_t port;
};

#endif // LIBBTCP2P_TYPES_H
