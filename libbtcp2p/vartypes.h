#ifndef LIBBTCP2P_VARTYPES_H
#define LIBBTCP2P_VARTYPES_H

#include <stdint.h>

#include "libbtcp2p/checked_buffer.h"
#include "libbtcp2p/types.h"

// btcp2p_varint_encode encodes value as a variable length integer.
void btcp2p_varint_encode(struct btcp2p_varint_t* const vi, uint64_t value);

// btcp2p_varint_pack pack a variable length integer into a checked buffer.
void btcp2p_varint_pack(struct btcp2p_varint_t const * const vi,
                        struct btcp2p_checked_buffer_t* cb);

// btcp2p_varint_unpack unpacks a variable length integer from a checked
// buffer.
bool btcp2p_varint_unpack(struct btcp2p_varint_t* vi,
                          struct btcp2p_checked_buffer_t* cb);

// btcp2p_varstr_encode encodes string as a variable length string. This
// function takes ownership of the value and places its pointer into the
// btcp2p_varstr_t structure given.
void btcp2p_varstr_encode(struct btcp2p_varstr_t* vs,
                          char * const value,
                          size_t value_size);

// btcp2p_varstr_pack packs a variable length string into the given checked
// buffer.
void btcp2p_varstr_pack(struct btcp2p_varstr_t const * const vs,
                        struct btcp2p_checked_buffer_t* cb);

// btcp2p_varstr_unpack unpacks a variable length string from a checked buffer.
bool btcp2p_varstr_unpack(struct btcp2p_varstr_t* vs,
                          struct btcp2p_checked_buffer_t* cb);

#endif // LIBBTCP2P_VARTYPES_H
