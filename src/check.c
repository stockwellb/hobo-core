#include "hobo/check.h"

static hobo_check_sink sink;

void hobo_check_begin(hobo_check_record *buf, size_t capacity) {
  sink.records = buf;
  sink.capacity = capacity;
  sink.count = 0;
}

bool hobo_check_failed(void) {
  size_t i;
  for (i = 0; i < sink.count; i++) {
    if (sink.records[i].passed == false) {
      return true;
    }
  }
  return false;
}

const hobo_check_sink *hobo_check_get(void) { return &sink; }

void test_check(bool passed, const char *expr, const char *file, int line) {
  if (sink.count == sink.capacity) {
    return;
  }

  hobo_check_record *slot = &sink.records[sink.count];
  slot->passed = passed;
  slot->expr = expr;
  slot->file = file;
  slot->line = line;
  slot->message = NULL;
  slot->detail = NULL;
  sink.count++;
}
