#include <signal.h>
#include <libbtcp2p/btcp2p.h>

static bool IsRunning = true;

void handle_ctrl_c(int _sig) {
  signal(SIGINT, handle_ctrl_c);
  IsRunning = false;
}

int main() {
  struct btcp2p_connection_t connection = { 0 };
  
  struct btcp2p_timer_t ping_timer = { 0 };
  btcp2p_timer_init(&ping_timer, 10 /* secs */);

  // Ensure that we clean up properly on SIGINT (ctrl-c)
  signal(SIGINT, handle_ctrl_c);
  
  if (btcp2p_connect(&connection, "testnet", "127.0.0.1")) {
    while (btcp2p_message_pump(&connection) && IsRunning) {
      if (btcp2p_has_message(&connection, "ping")) {
        uint64_t nonce;
        btcp2p_unpack_message(&connection, "l", &nonce);
        btcp2p_pack_and_send_message(&connection, "pong", "l", nonce);
      }

      if (btcp2p_timer_expired(&ping_timer)) {
        btcp2p_log(BTCP2P_LOG_INFO, "timer expired, sending ping.\n");
        btcp2p_pack_and_send_message(&connection, "ping", "o");
        btcp2p_timer_reset(&ping_timer);
      }

      /* Any message */
      if (btcp2p_has_message(&connection, NULL)) {
        btcp2p_log(BTCP2P_LOG_INFO, "got message %.*s\n", 12, connection.message.header.command);
      }
    }

    btcp2p_disconnect(&connection);
  }
  
  return 0;
}
