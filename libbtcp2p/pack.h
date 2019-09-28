// Provides general interfaces for packing and unpacking various types of
// Bitcoin P2P protocol types into buffers or unpacking them from buffers.
//
// Example:
//   btcp2p_pack(&cb, "bbI", 0xFF, 0xFF, -1234);
//
// Format Strings:
//   8-bit integer:
//     b - unsigned (uint8_t)
//     B - signed (int8_t)
//   16-bit integer:
//     s - unsigned (uint16_t)
//     S - signed (int16_t)
//   32-bit integer:
//     i - unsigned (uint32_t)
//     I - signed (int32_t)
//   64-bit integer:
//     l - unsigned (uint64_t)
//     L - signed (int64_t)
//   Variable length:
//     v - integer (btcp2p_varint_t)
//     j - string (btcp2p_varstr_t)
//   Network address:
//     n - with timestamp (btcp2p_netaddr_t)
//     N - without timestamp (btcp2p_netaddr_t)
//   Others:
//     o - generate and pack 64-bit nonce.
//     h - 32-byte hash (char[32])
#ifndef LIBBTCP2P_PACK_H
#define LIBBTCP2P_PACK_H

#include <stdarg.h>

#include "libbtcp2p/checked_buffer.h"

// btcp2p_pack packs a message of the given format into a checked buffer from
// arguments.
size_t btcp2p_pack(struct btcp2p_checked_buffer_t* cb,
                   char const * const restrict format,
                   ...);

// btcp2p_pack same as btcp2p_pack but takes a va_list of arguments to pack.
size_t btcp2p_vpack(struct btcp2p_checked_buffer_t* cb,
                    char const * const restrict format,
                    va_list args);

// btcp2p_unpack unpacks a message of the given format from the checked buffer
// into arguments.
size_t btcp2p_unpack(struct btcp2p_checked_buffer_t* cb,
                     char const * const restrict format,
                     ...);

// btcp2p_vunpack same as btcp2p_unpack but takes a va_list of arguments to
// unpack.
size_t btcp2p_vunpack(struct btcp2p_checked_buffer_t* cb,
                      char const * const restrict format,
                      va_list args);

#endif // LIBBTCP2P_PACK_H
