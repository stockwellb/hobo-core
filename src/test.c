#include "hobo/test.h"
#include "hobo/check.h"
#include <stddef.h>
#include <stdio.h>

static hobo_check_record records[64];

int hobo_test_run_suite(hobo_test_suite *suite) {
  printf("TAP version 13\n");

  size_t count = 0;
  for (hobo_test_case *test = suite->tests; test->run != NULL; test++) {
    count++;
  }

  printf("1..%zu\n", count);

  void *ctx = NULL;
  if (suite->suite_setup != NULL) {
    ctx = suite->suite_setup();
  }

  size_t failed = 0;
  size_t test_number = 1;

  for (hobo_test_case *test = suite->tests; test->run != NULL; test++) {
    hobo_check_begin(records, sizeof records / sizeof records[0]);

    if (test->setup != NULL) {
      test->setup(ctx);
    }

    bool result = test->run(ctx);

    if (test->teardown != NULL) {
      test->teardown(ctx);
    }

    if (result) {
      printf("ok %zu - %s\n", test_number, test->name);
    } else {
      failed++;
      printf("not ok %zu - %s\n", test_number, test->name);
    }

    const hobo_check_sink *sink = hobo_check_get();

    for (size_t i = 0; i < sink->count; i++) {
      const hobo_check_record *record = &sink->records[i];

      if (!record->passed) {
        printf("# %s - %s:%d\n", record->expr, record->file, record->line);
      }
    }

    test_number++;
  }

  if (suite->suite_teardown != NULL) {
    suite->suite_teardown(ctx);
  }

  return failed != 0;
}
