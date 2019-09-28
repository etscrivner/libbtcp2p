// Bitcoin P2P protocol-specific types
#ifndef LIBBTCP2P_TYPES_H
#define LIBBTCP2P_TYPES_H

#include <stdint.h>

// Allow for a maximum of 1KB of variable string data.
#define MAX_VARSTR_LENGTH 1024

// Variable-length integer type (up to 64-bits)
struct btcp2p_varint_t {
  uint8_t length; ///< The length of the data field.
  uint8_t data[9]; ///< Contains packed variable length integer data
  uint64_t value; ///< The actual value that was packed
};

// Variable length string interface
struct btcp2p_varstr_t {
  struct btcp2p_varint_t length; ///< Length of the string in bytes.
  // TODO: Set a max length for varstr's and change this to a static,
  // fixed-length string. This routes around allocation issues.
  char* data; ///< Actual string data
};

// Network address type
struct btcp2p_netaddr_t {
  uint32_t time; ///< Timestamp on the address..
  uint64_t services; ///< Advertises services provided by this address.
  char address[16]; ///< The address itself in IPV6 network format.
  uint16_t port; ///< Port on which the address provides bitcoin services.
};


#endif // LIBBTCP2P_TYPES_H
