#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "acutest.h"

#include <libbtcp2p/checked_buffer.h>

void test_sanity_check() {
  struct btcp2p_checked_buffer_t cb;
  btcp2p_checked_buffer_create(&cb);
  btcp2p_checked_buffer_destroy(&cb);
}

void test_checked_read_resize() {
  char* data = "hello";

  struct btcp2p_checked_buffer_t cb;
  memset(&cb, 0, sizeof(struct btcp2p_checked_buffer_t));
  btcp2p_checked_buffer_resize(&cb, 2);
  btcp2p_checked_buffer_prepare_read(&cb, (uint8_t*)data, strlen(data));

  TEST_CHECK(cb.capacity >= strlen(data));
  TEST_CHECK(strncmp((char*)cb.buffer, data, cb.len) == 0);

  btcp2p_checked_buffer_destroy(&cb);
}

void test_checked_reads() {
  uint32_t expected[2] = {
    0x12345678,
    0xABCDEF12
  };
  uint32_t actual[2];

  struct btcp2p_checked_buffer_t cb;
  memset(&cb, 0, sizeof(struct btcp2p_checked_buffer_t));
  btcp2p_checked_buffer_resize(&cb, 64);

  btcp2p_checked_buffer_prepare_read(
    &cb, (uint8_t*)expected, sizeof(uint32_t) * 2
  );

  TEST_CHECK(btcp2p_checked_buffer_read(&cb, (uint8_t*)&actual, sizeof(uint32_t) * 2));
  TEST_CHECK(memcmp(expected, actual, sizeof(uint32_t) * 2) == 0);
  TEST_CHECK(!btcp2p_checked_buffer_read(&cb, (uint8_t*)&actual[0], sizeof(uint32_t)));

  btcp2p_checked_buffer_destroy(&cb);
}

void test_checked_writes() {
  struct btcp2p_checked_buffer_t cb;
  memset(&cb, 0, sizeof(struct btcp2p_checked_buffer_t));
  btcp2p_checked_buffer_resize(&cb, 4);

  uint32_t value = 0x12345678;
  btcp2p_checked_buffer_prepare_write(&cb);
  btcp2p_checked_buffer_write(&cb, (uint8_t*)&value, sizeof(uint32_t));
  btcp2p_checked_buffer_write(&cb, (uint8_t*)&value, sizeof(uint32_t));

  TEST_CHECK(cb.capacity > 4);

  btcp2p_checked_buffer_destroy(&cb);
}

void fuzz_checked_buffer() {
  // TODO: implement this
  // Randomly read and write various amounts of data in an attempt to crash
  // the program.
}

TEST_LIST = {
  { "test_sanity_check", test_sanity_check },
  { "test_dynamic_resize", test_checked_read_resize },
  { "test_checked_reads", test_checked_reads },
  { "test_checked_writes", test_checked_writes },
  { "fuzz_checked_buffer", fuzz_checked_buffer },
  { 0 },
};
