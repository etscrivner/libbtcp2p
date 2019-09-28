// Simple timer interfaces
#ifndef LIBBTCP2P_TIMER_H
#define LIBBTCP2P_TIMER_H

#include <stdbool.h>
#include <time.h>

struct btcp2p_timer_t {
  time_t previous_time;
  double timeout;
};

// btcp2p_timer_init initialized the given timer for the given timeout.
void btcp2p_timer_init(struct btcp2p_timer_t* const timer, double timeout);

// btcp2p_timer_reset resets the given timer to start counting from zero.
void btcp2p_timer_reset(struct btcp2p_timer_t* const timer);

// btcp2p_timer_expired return true if the given timer's timeout has been
// reached.
bool btcp2p_timer_expired(struct btcp2p_timer_t const * const timer);

#endif // LIBBTCP2P_TIMER_H
