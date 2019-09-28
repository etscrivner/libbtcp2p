#include "libbtcp2p/vartypes.h"

void btcp2p_varint_encode(struct btcp2p_varint_t* const vi, uint64_t value) {
  vi->value = value;

  if (value < 0xFD) {
    vi->length = 1;
    vi->data[0] = value;
  } else if (value <= 0xFFFF) {
    vi->length = 3;
    vi->data[0] = 0xFD;
    *((uint16_t*)(&vi->data[1])) = value;
  } else if (value <= 0xFFFFFFFF) {
    vi->length = 5;
    vi->data[0] = 0xFE;
    *((uint32_t*)(&vi->data[1])) = value;
  } else {
    vi->length = 9;
    vi->data[0] = 0xFF;
    *((uint64_t*)(&vi->data[1])) = value;
  }
}

void btcp2p_varint_pack(struct btcp2p_varint_t const * const vi,
                        struct btcp2p_checked_buffer_t* cb)
{
  btcp2p_checked_buffer_write(cb, vi->data, vi->length);
}

bool btcp2p_varint_unpack(struct btcp2p_varint_t* vi,
                          struct btcp2p_checked_buffer_t* cb)
{
  // NOTE: Code below will not work on big-endian machines.

  // First, read the leading byte which will tell us the total data size.
  if (!btcp2p_checked_buffer_read(cb, &vi->data[0], 1)) {
    return false;
  }

  if (vi->data[0] < 0xFD) {
    vi->length = 1;
    vi->value = vi->data[0];
  } else if (vi->data[0] == 0xFD) {
    vi->length = 3;
    if (!btcp2p_checked_buffer_read(cb, &vi->data[1], 2)) {
      return false;
    }
    vi->value = *(uint16_t*)&vi->data[1];
  } else if (vi->data[0] == 0xFE) {
    vi->length = 5;
    if (!btcp2p_checked_buffer_read(cb, &vi->data[1], 4)) {
      return false;
    }
    vi->value = *(uint32_t*)&vi->data[1];
  } else {
    vi->length = 9;
    if (!btcp2p_checked_buffer_read(cb, &vi->data[1], 8)) {
      return false;
    }
    vi->value = *(uint64_t*)&vi->data[1];
  }

  return true;
}

void btcp2p_varstr_encode(struct btcp2p_varstr_t* vs,
                          char * const value,
                          size_t value_size)
{
  btcp2p_varint_encode(&vs->length, value_size);
  vs->data = value;
}

void btcp2p_varstr_pack(struct btcp2p_varstr_t const * const vs,
                        struct btcp2p_checked_buffer_t* cb)
{
  btcp2p_varint_pack(&vs->length, cb);
  btcp2p_checked_buffer_write(cb, (uint8_t*)vs->data, vs->length.value);
}

bool btcp2p_varstr_unpack(struct btcp2p_varstr_t* vs,
                          struct btcp2p_checked_buffer_t* cb)
{
  if (!btcp2p_varint_unpack(&vs->length, cb)) {
    return false;
  }

  if (!btcp2p_checked_buffer_has_readable_bytes(cb, vs->length.value)) {
    return false;
  }

  // TODO: We literally just point to the contents of the buffer at this point.
  // Any calling code that actually wants to use the string should strdup() it
  // first. This is really nasty and non-obvious... perhaps there's a better
  // way.
  vs->data = (char*)btcp2p_checked_buffer_cursor(cb);

  if (!btcp2p_checked_buffer_fastforward(cb, vs->length.value)) {
    return false;
  }

  return true;
}
