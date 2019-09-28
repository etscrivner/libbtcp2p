// Simple logging interfaces
#ifndef LIBBTCP2P_LOG_H
#define LIBBTCP2P_LOG_H

#include <stdint.h>
#include <stdlib.h>

// Logging levels
enum btcp2p_log_level_t {
  BTCP2P_LOG_DEBUG,
  BTCP2P_LOG_INFO,
  BTCP2P_LOG_ERROR
};

// btcp2p_log logs a message of the given level to stdout.
void btcp2p_log(enum btcp2p_log_level_t log_level,
                char const * const restrict format,
                ...);

// btcp2p_log_dump dumps a string of bytes as hex to stdout.
void btcp2p_log_dump(enum btcp2p_log_level_t log_level,
                     size_t value_size,
                     uint8_t const value[value_size]);

#endif // LIBBTCP2P_LOG_H
