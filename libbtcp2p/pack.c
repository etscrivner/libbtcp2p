#include <stdarg.h>
#include <stdio.h>

#include <openssl/rand.h>

#include "libbtcp2p/log.h"
#include "libbtcp2p/pack.h"
#include "libbtcp2p/types.h"
#include "libbtcp2p/vartypes.h"

size_t btcp2p_pack(struct btcp2p_checked_buffer_t* cb,
                   char const * const restrict format,
                   ...)
{
  va_list args;
  va_start(args, format);
  size_t size = btcp2p_vpack(cb, format, args);
  va_end(args);

  return size;
}

size_t btcp2p_vpack(struct btcp2p_checked_buffer_t* cb,
                    char const * const restrict format,
                    va_list args)
{
  uint8_t b;
  int8_t B;
  uint16_t s;
  int16_t S;
  uint32_t i;
  int32_t I;
  uint64_t l;
  int64_t L;
  char* hash;

  struct btcp2p_varint_t varint;
  struct btcp2p_varstr_t varstr;
  struct btcp2p_netaddr_t netaddr;

  char const* next = format;
  while (*next != '\0') {
    switch (*next++) {
    case 'b':
      {
        b = va_arg(args, int);
        btcp2p_checked_buffer_write(cb, &b, sizeof(uint8_t));
      }
      break;
    case 'B':
      {
        B = va_arg(args, int);
        btcp2p_checked_buffer_write(cb, (uint8_t*)&B, sizeof(int8_t));
      }
      break;
    case 's':
      {
        s = va_arg(args, int);
        btcp2p_checked_buffer_write(cb, (uint8_t*)&s, sizeof(uint16_t));
      }
      break;
    case 'S':
      {
        S = va_arg(args, int);
        btcp2p_checked_buffer_write(cb, (uint8_t*)&S, sizeof(int16_t));
      }
      break;
    case 'i':
      {
        i = va_arg(args, uint32_t);
        btcp2p_checked_buffer_write(cb, (uint8_t*)&i, sizeof(uint32_t));
      }
      break;
    case 'I':
      {
        I = va_arg(args, int32_t);
        btcp2p_checked_buffer_write(cb, (uint8_t*)&I, sizeof(int32_t));
      }
      break;
    case 'l':
      {
        l = va_arg(args, uint64_t);
        btcp2p_checked_buffer_write(cb, (uint8_t*)&l, sizeof(uint64_t));
      }
      break;
    case 'L':
      {
        L = va_arg(args, int64_t);
        btcp2p_checked_buffer_write(cb, (uint8_t*)&L, sizeof(int64_t));
      }
      break;
    case 'v':
      {
        varint = va_arg(args, struct btcp2p_varint_t);
        btcp2p_varint_pack(&varint, cb);
      }
      break;
    case 'j':
      {
        varstr = va_arg(args, struct btcp2p_varstr_t);
        btcp2p_varstr_pack(&varstr, cb);
      }
      break;
    case 'o':
      {
        RAND_bytes((uint8_t*)&l, sizeof(uint64_t));
        btcp2p_checked_buffer_write(cb, (uint8_t*)&l, sizeof(uint64_t));
      }
      break;
    case 'n':
      {
        netaddr = va_arg(args, struct btcp2p_netaddr_t);
        btcp2p_checked_buffer_write(cb, (uint8_t*)&netaddr.time, sizeof(uint32_t));
        goto pack_netaddr_common;
      }
    case 'N':
      {
        netaddr = va_arg(args, struct btcp2p_netaddr_t);
      pack_netaddr_common:
        btcp2p_checked_buffer_write(cb, (uint8_t*)&netaddr.services, sizeof(uint64_t));
        btcp2p_checked_buffer_write(cb, (uint8_t*)&netaddr.address, 16);
        btcp2p_checked_buffer_write(cb, (uint8_t*)&netaddr.port, sizeof(uint16_t));
      }
      break;
    case 'h':
      {
        hash = va_arg(args, char*);
        btcp2p_checked_buffer_write(cb, (uint8_t*)hash, 32);
      }
      break;
    default:
      break;
    }
  }

  return cb->rw_cursor;
}

#define BTCP2P_UNPACK_TYPE(CB, DST, TYPE) {                           \
  if (!btcp2p_checked_buffer_read(CB, (uint8_t*)DST, sizeof(TYPE)))   \
  { goto loop_done; }                                                 \
}
#define BTCP2P_UNPACK_SIMPLE(CB, TYPE) BTCP2P_UNPACK_TYPE(CB, va_arg(args, TYPE*), TYPE)
#define BTCP2P_UNPACK_COMPLEX(CB, DST, SIZE) {                  \
    if (!btcp2p_checked_buffer_read(CB, (uint8_t*)DST, SIZE))   \
    { goto loop_done; }                                         \
}                     

size_t btcp2p_unpack(struct btcp2p_checked_buffer_t* cb,
                     char const * const restrict format,
                     ...)
{
  va_list args;
  va_start(args, format);
  size_t size = btcp2p_vunpack(cb, format, args);
  va_end(args);

  return size;
}

size_t btcp2p_vunpack(struct btcp2p_checked_buffer_t* cb,
                      char const * const restrict format,
                      va_list args)
{
  struct btcp2p_netaddr_t* netaddr;
  struct btcp2p_varint_t* varint;
  struct btcp2p_varstr_t* varstr;

  char const* next = format;
  while (*next != '\0') {
    switch (*next++) {
    case 'b':
      BTCP2P_UNPACK_SIMPLE(cb, uint8_t);
      break;
    case 'B':
      BTCP2P_UNPACK_SIMPLE(cb, int8_t);
      break;
    case 's':
      BTCP2P_UNPACK_SIMPLE(cb, uint16_t);
      break;
    case 'S':
      BTCP2P_UNPACK_SIMPLE(cb, int16_t);
      break;
    case 'i':
      BTCP2P_UNPACK_SIMPLE(cb, uint32_t);
      break;
    case 'I':
      BTCP2P_UNPACK_SIMPLE(cb, int32_t);
      break;
    case 'o':
    case 'l':
      BTCP2P_UNPACK_SIMPLE(cb, uint64_t);
      break;
    case 'L':
      BTCP2P_UNPACK_SIMPLE(cb, int64_t);
      break;
    case 'v':
      {
        varint = (struct btcp2p_varint_t*)va_arg(args, struct btcp2p_varint_t*);
        if (!btcp2p_varint_unpack(varint, cb)) { goto loop_done; }
      }
      break;
    case 'j':
      {
        varstr = (struct btcp2p_varstr_t*)va_arg(args, struct btcp2p_varstr_t*);
        if (!btcp2p_varstr_unpack(varstr, cb)) { goto loop_done; }
      }
      break;
    case 'n':
      {
        netaddr = (struct btcp2p_netaddr_t*)va_arg(args, struct btcp2p_netaddr_t*);
        BTCP2P_UNPACK_TYPE(cb, &netaddr->time, uint32_t);
        goto unpack_netaddr_common;
      }
    case 'N':
      {
        netaddr = (struct btcp2p_netaddr_t*)va_arg(args, struct btcp2p_netaddr_t*);
        // We have this label to share common code between netaddr's with and
        // without timestamps.
      unpack_netaddr_common:
        BTCP2P_UNPACK_TYPE(cb, &netaddr->services, uint64_t);
        BTCP2P_UNPACK_COMPLEX(cb, &netaddr->address, 16);
        BTCP2P_UNPACK_TYPE(cb, &netaddr->port, uint16_t);
      }
      break;
    case 'h':
      {
        BTCP2P_UNPACK_COMPLEX(cb, va_arg(args, char*), 32);
      }
      break;
    default:
      break;
    }
  }
  // Abort to this label if ever there is an attempt to read beyond the
  // available data in the checked buffer.
 loop_done:

  return cb->rw_cursor;
}
