#ifndef HOBO_TEST_H
#define HOBO_TEST_H

#include "hobo/check.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

typedef enum {
  HOBO_TEST_PASS,
  HOBO_TEST_FAIL,
  HOBO_TEST_SKIP,
} hobo_test_kind;

typedef struct hobo_test_result {
  hobo_test_kind kind;
  const char *skip_reason;
  const hobo_check_sink *checks;
} hobo_test_result;

typedef struct hobo_test_summary {
  size_t total;
  size_t passed;
  size_t failed;
  size_t skipped;
} hobo_test_summary;

typedef struct {
  const char *name;
  void (*setup)(void *ctx);
  bool (*run)(void *ctx);
  void (*teardown)(void *ctx);
  const char *skip;

} hobo_test_case;

typedef struct {
  const char *name;
  void *(*suite_setup)(void);
  hobo_test_case *tests;
  void (*suite_teardown)(void *ctx);
} hobo_test_suite;

typedef struct hobo_reporter {
  void *state;
  void (*suite_begin)(void *state, const hobo_test_suite *suite,
                      size_t test_count);
  void (*test_end)(void *state, const hobo_test_case *test,
                   const hobo_test_result *result);
  void (*suite_end)(void *state, const hobo_test_summary *summary);
} hobo_reporter;

int hobo_test_run_suite(hobo_test_suite *suite, const hobo_reporter *reporter);

#endif
