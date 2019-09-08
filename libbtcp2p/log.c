#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#include "libbtcp2p/log.h"

static char* btcp2p_log_timestamp(time_t t) {
  struct tm* utc_time;
  static char date[100];
 
  utc_time = gmtime(&t);
  strftime(date, 100, "%Y-%M-%dT%H:%M:%S%z", utc_time);
 
  return date;
}

static const char* btcp2p_log_level_str(enum btcp2p_log_level_t log_level) {
  switch (log_level) {
  case BTCP2P_LOG_DEBUG:
    return "debug";
  case BTCP2P_LOG_INFO:
    return "info";
  case BTCP2P_LOG_ERROR:
    return "error";
  default:
    return "unknown";
  }
}

void btcp2p_log(enum btcp2p_log_level_t log_level,
                char const * const restrict format,
                ...)
{
  time_t now;
  time(&now);

  va_list args;
  va_start(args, format);

  printf("btcp2p[%s]: %s: ", btcp2p_log_timestamp(now), btcp2p_log_level_str(log_level));
  vprintf(format, args);

  va_end(args);
}

void btcp2p_log_dump(enum btcp2p_log_level_t log_level,
                     size_t value_size,
                     uint8_t const value[value_size])
{
  time_t now;
  time(&now);

  printf("btcp2p[%s]: %s: hex dump\n", btcp2p_log_timestamp(now), btcp2p_log_level_str(log_level));
  for (size_t i = 0; i < value_size; i++) {
    printf("%02X ", value[i]);
  }
  printf("\n");
}
