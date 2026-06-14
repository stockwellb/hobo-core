#ifndef HOBO_CHECK_H
#define HOBO_CHECK_H
#include <stdbool.h>
#include <stddef.h>

typedef struct {
  bool passed;
  const char *expr;
  const char *message;
  const char *detail;
  const char *file;
  int line;
} hobo_check_record;

typedef struct {
  hobo_check_record *records;
  size_t count;
  size_t capacity;
} hobo_check_sink;

void hobo_check_begin(hobo_check_record *buf, size_t capacity);
bool hobo_check_failed(void);
const hobo_check_sink *hobo_check_get(void);

void test_check(bool passed, const char *expr, const char *file, int line);

#define HOBO_CHECK_RESULT() (hobo_check_failed() ? TEST_FAIL : TEST_PASS)
#define CHECK(cond) test_check(!!(cond), #cond, __FILE__, __LINE__)

#endif
