#include <stdlib.h>
#include <string.h>

#include "libbtcp2p/checked_buffer.h"

// TODO: Do we really need this? Premature optimization?

// nearest_power_of_two does some bit-twiddling to get the nearest power of 2
// to the given value.
uint32_t nearest_power_of_two(uint32_t v) {
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
}

void btcp2p_checked_buffer_create(struct btcp2p_checked_buffer_t* cb) {
  memset(cb, 0, sizeof(struct btcp2p_checked_buffer_t));
  btcp2p_checked_buffer_resize(cb, BTCP2P_CHECKED_BUFFER_INITIAL_CAPACITY);
}

void btcp2p_checked_buffer_resize(struct btcp2p_checked_buffer_t* cb,
                                  size_t capacity)
{
  // TODO: Test for overflows when finding nearest power of 2.
  cb->capacity = nearest_power_of_two(capacity);
  cb->buffer = realloc(cb->buffer, cb->capacity);
}

void btcp2p_checked_buffer_destroy(struct btcp2p_checked_buffer_t* cb) {
  free(cb->buffer);
  cb->buffer = NULL;
  cb->len = 0;
  cb->capacity = 0;
  cb->rw_cursor = 0;
}

bool btcp2p_checked_buffer_has_readable_bytes(struct btcp2p_checked_buffer_t* cb,
                                              size_t length)
{
  return (cb->len - cb->rw_cursor) >= length;
}

bool btcp2p_checked_buffer_fastforward(struct btcp2p_checked_buffer_t* cb,
                                       size_t offset)
{
  if (!btcp2p_checked_buffer_has_readable_bytes(cb, offset)) {
    return false;
  }

  cb->rw_cursor += offset;
  return true;
}

void btcp2p_checked_buffer_read_reset(struct btcp2p_checked_buffer_t* cb)
{
  cb->rw_cursor = 0;
}

void btcp2p_checked_buffer_prepare_read(struct btcp2p_checked_buffer_t* cb,
                                        uint8_t const * const src,
                                        size_t src_len)
{
  if (src_len > cb->capacity) {
    btcp2p_checked_buffer_resize(cb, src_len);
  }

  memcpy(cb->buffer, src, src_len);
  cb->len = src_len;
  cb->rw_cursor = 0;
}

bool btcp2p_checked_buffer_read(struct btcp2p_checked_buffer_t* cb,
                                uint8_t * const dst,
                                size_t read_amount)
{
  // We don't have sufficient data in the buffer to satisfy the read.
  if (read_amount > (cb->len - cb->rw_cursor)) {
    return false;
  }

  memcpy(dst, (cb->buffer + cb->rw_cursor), read_amount);
  cb->rw_cursor += read_amount;

  return true;
}

void btcp2p_checked_buffer_prepare_write(struct btcp2p_checked_buffer_t* cb) {
  cb->rw_cursor = 0;
  cb->len = 0;
}

void btcp2p_checked_buffer_write(struct btcp2p_checked_buffer_t* cb,
                                 uint8_t const * const src,
                                 size_t write_amount)
{
  uint32_t remaining_space = cb->capacity - cb->rw_cursor;
  if (write_amount > remaining_space) {
    btcp2p_checked_buffer_resize(cb, cb->capacity + (write_amount - remaining_space));
  }

  memcpy(cb->buffer + cb->rw_cursor, src, write_amount);
  cb->rw_cursor += write_amount;
}

uint32_t btcp2p_checked_buffer_amount_written(struct btcp2p_checked_buffer_t* cb) {
  return cb->rw_cursor;
}
