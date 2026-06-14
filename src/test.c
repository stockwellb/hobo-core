#include "hobo/test.h"
#include "hobo/check.h"
#include <stddef.h>
#include <stdio.h>

static hobo_check_record records[64];

int hobo_test_run_suite(hobo_test_suite *suite) {
  printf("TAP version 13\n");

  int count = 0;
  hobo_test_case *tp = suite->tests;

  while (tp->run != NULL) {
    count++;
    tp++;
  };

  printf("1..%d\n", count);

  void *ctx = NULL;

  // suite setup
  if (suite->suite_setup != NULL) {
    ctx = suite->suite_setup();
  }

  unsigned int failed = 0;

  hobo_test_case *t = suite->tests;
  unsigned n = 1;

  while (t->run != NULL) {

    hobo_check_begin(records, sizeof records / sizeof records[0]);

    // setup
    if (t->setup != NULL) {
      t->setup(ctx);
    }

    // run test
    bool result = t->run(ctx);

    // teardown
    if (t->teardown != NULL) {
      t->teardown(ctx);
    }

    // render
    if (result) {
      printf("ok %u - %s\n", n, t->name);
    } else {
      failed++;
      printf("not ok %u - %s\n", n, t->name);
    }

    const hobo_check_sink *sink = hobo_check_get();

    size_t i;
    for (i = 0; i < sink->count; i++) {
      const hobo_check_record *r = &sink->records[i];
      if (!r->passed) {
        printf("# %s - %s:%d\n", r->expr, r->file, r->line);
      }
    }

    // next
    n++;
    t++;
  }

  // suite teardown
  if (suite->suite_teardown != NULL) {
    suite->suite_teardown(ctx);
  }

  return failed != 0;
}
