#include "hobo/test.h"
#include "hobo/check.h"
#include <stdio.h>

int hobo_test_run_suite(hobo_test_suite *suite) {
  printf("=== %s ===\n", suite->name);

  void *ctx = NULL;

  // suite setup
  if (suite->suite_setup != NULL) {
    ctx = suite->suite_setup();
  }

  unsigned int passed = 0, failed = 0, skipped = 0;

  hobo_test_case *t = suite->tests;
  while (t->run != NULL) {
    printf("--- %s ---\n", t->name);
    // check for skipped
    if (t->skip) {
      printf("[SKIP]\n");
      skipped++;
      continue;
    };

    HOBO_CHECK_RESET();

    // setup
    if (t->setup != NULL) {
      t->setup(ctx);
    }

    // run test
    hobo_test_result result = t->run(ctx);

    // teardown
    if (t->teardown != NULL) {
      t->teardown(ctx);
    }

    // evaluate result
    switch (result) {
    case TEST_PASS:
      passed++;
      break;
    case TEST_FAIL:
      failed++;
      break;
    case TEST_SKIP:
      skipped++;
      break;
    }

    // next
    t++;
  }

  // suite teardown
  if (suite->suite_teardown != NULL) {
    suite->suite_teardown(ctx);
  }

  printf("passed: %d, failed %d, skipped %d\n", passed, failed, skipped);
  return failed != 0;
}
