// Provides general interfaces for packing and unpacking various types of
// Bitcoin P2P protocol types into buffers or unpacking them from buffers.
//
// Example:
//   btcp2p_pack(&cb, "bbI", 0xFF, 0xFF, -1234);
//
// Format Strings:
//   8-bit integer:
//     b - unsigned
//     B - signed
//   16-bit integer:
//     s - unsigned
//     S - signed
//   32-bit integer:
//     i - unsigned
//     I - signed
//   64-bit integer:
//     l - unsigned
//     L - signed
//   Variable length:
//     v - integer (should be uint64_t)
//     j - string
//   Network address:
//     n - with timestamp
//     N - without timestamp
//   Others:
//     o - 64-bit nonce value (Nonce is generated for you on pack)
//     h - 32-byte hash
#ifndef LIBBTCP2P_PACK_H
#define LIBBTCP2P_PACK_H

// TODO: Add hash types

#include <stdarg.h>

#include "libbtcp2p/checked_buffer.h"

size_t btcp2p_pack(struct btcp2p_checked_buffer_t* cb,
                   char const * const restrict format,
                   ...);

size_t btcp2p_vpack(struct btcp2p_checked_buffer_t* cb,
                    char const * const restrict format,
                    va_list args);

size_t btcp2p_unpack(struct btcp2p_checked_buffer_t* cb,
                     char const * const restrict format,
                     ...);

size_t btcp2p_vunpack(struct btcp2p_checked_buffer_t* cb,
                      char const * const restrict format,
                      va_list args);

#endif // LIBBTCP2P_PACK_H
