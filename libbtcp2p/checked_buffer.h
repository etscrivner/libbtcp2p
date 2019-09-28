// Implements a dynamically growable buffer with checked reads and writes.
//
// This buffer is designed for the needs of dynamically packing and unpacking
// payloads received/sent over the network and is not intended to be a
// general-purpose data structure.
#ifndef LIBBTCP2P_CHECKED_BUFFER_H
#define LIBBTCP2P_CHECKED_BUFFER_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

// Initial capacity allocated to the checked buffer.
#define BTCP2P_CHECKED_BUFFER_INITIAL_CAPACITY 1024

struct btcp2p_checked_buffer_t {
  uint8_t* buffer;
  size_t len;       //< number of bytes written in the buffer.
  size_t rw_cursor; //< index of the end of the last read or write.
  size_t capacity;  //< total bytes allocated.
};

// btcp2p_checked_buffer_init initializes a new empty checked buffer
void btcp2p_checked_buffer_create(struct btcp2p_checked_buffer_t* cb);

// btcp2p_checked_buffer_resize resizes the given checked buffer or creates a new
// checked buffer with the given initial capacity.
void btcp2p_checked_buffer_resize(struct btcp2p_checked_buffer_t* cb,
                                  size_t capacity);

// btcp2p_checked_buffer_destroy free resources allocated for the checked buffer.
void btcp2p_checked_buffer_destroy(struct btcp2p_checked_buffer_t* cb);

// btcp2p_checked_buffer_has_readable_bytes indicates whether or not at least
// the given number of bytes can be read from the buffer.
bool btcp2p_checked_buffer_has_readable_bytes(struct btcp2p_checked_buffer_t* cb,
                                              size_t length);

// btcp2p_checked_buffer_fastforward moves the read/write offset forward by the
// given number of bytes.
bool btcp2p_checked_buffer_fastforward(struct btcp2p_checked_buffer_t* cb,
                                       size_t offset);

void btcp2p_checked_buffer_read_reset(struct btcp2p_checked_buffer_t* cb);

// btcp2p_checked_buffer_prepare_read loads the given data into the checked
// buffer for reading, resizing as needed. It then resets the read/write cursor
// to the start of the buffer.
void btcp2p_checked_buffer_prepare_read(struct btcp2p_checked_buffer_t* cb,
                                        uint8_t const * const src,
                                        size_t src_len);

// btcp2p_checked_buffer_read reads the given amount of data from the buffer
// into the destination buffer. It returns true if the amount was successfully
// read or false if there was insufficient data in the buffer to satisfy the
// read.
bool btcp2p_checked_buffer_read(struct btcp2p_checked_buffer_t* cb,
                                uint8_t * const dst,
                                size_t read_amount);

// btcp2p_checked_buffer_prepare_write resets the read/write cursor so that
// writing can start from the beginning of the buffer.
void btcp2p_checked_buffer_prepare_write(struct btcp2p_checked_buffer_t* cb);

// btcp2p_checked_buffer_write writes the given amount of data from the source
// buffer into the destination buffer. It resizes the buffer if there is an
// attempt to write beyond the existing capacity.
void btcp2p_checked_buffer_write(struct btcp2p_checked_buffer_t* cb,
                                 uint8_t const * const src,
                                 size_t write_amount);

// btcp2p_checked_buffer_amount_written returns the number of bytes writtenp
// into the internal buffer.
uint32_t btcp2p_checked_buffer_amount_written(struct btcp2p_checked_buffer_t* cb);

// btcp2p_checked_buffer_prepare_copy prepares the given checked buffer to copy
// the given amount of data. It returns a pointer to a buffer that can receive
// the given amount of data or NULL if enough space could not be allocated.
uint8_t* btcp2p_checked_buffer_prepare_copy(struct btcp2p_checked_buffer_t* cb,
                                            size_t copy_amount_bytes);

// btcp2p_checked_buffer_pointer returns a pointer to the checked buffer at its
// current read-write position.
uint8_t* btcp2p_checked_buffer_cursor(struct btcp2p_checked_buffer_t* cb);

#endif // LIBBTCP2P_CHECKED_BUFFER_H
