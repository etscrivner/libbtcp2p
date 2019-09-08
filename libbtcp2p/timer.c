#include "libbtcp2p/timer.h"

void btcp2p_timer_init(struct btcp2p_timer_t* const timer, double timeout) {
  timer->previous_time = time(NULL);
  timer->timeout = timeout;
}

void btcp2p_timer_reset(struct btcp2p_timer_t* const timer) {
  timer->previous_time = time(NULL);
}

bool btcp2p_timer_expired(struct btcp2p_timer_t const * const timer) {
  if (difftime(time(NULL), timer->previous_time) >= timer->timeout) {
    return true;
  }

  return false;
}
