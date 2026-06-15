#include <hobo/check.h>
#include <hobo/tap_reporter.h>
#include <hobo/test.h>
#include <stddef.h>

/** Per-test check capacity.
 * Reset at the start of every test.
 * This is the max CHECKs in a SINGLE test, not the whole suite.
 * 256 is deliberately generous.
 * Tests with more than 256 checks should be split.
 * Any overflow is dropped.
 */
static hobo_check_record records[256];

int hobo_test_run_suite(hobo_test_suite *suite, const hobo_reporter *reporter) {
  size_t count = 0;
  for (hobo_test_case *test = suite->tests; test->run != NULL; test++) {
    count++;
  }

  reporter->suite_begin(reporter->state, suite, count);

  void *ctx = NULL;
  if (suite->suite_setup != NULL) {
    ctx = suite->suite_setup();
  }

  size_t passed = 0;
  size_t failed = 0;
  size_t skipped = 0;

  for (hobo_test_case *test = suite->tests; test->run != NULL; test++) {
    hobo_check_begin(records, sizeof records / sizeof records[0]);

    hobo_test_result result;

    if (test->skip) {
      result.kind = HOBO_TEST_SKIP;
      result.skip_reason = test->skip;
      result.checks = NULL;
      skipped++;
      reporter->test_end(reporter->state, test, &result);
      continue;
    }

    if (test->setup != NULL) {
      test->setup(ctx);
    }

    bool ok = test->run(ctx);

    if (test->teardown != NULL) {
      test->teardown(ctx);
    }

    if (ok && !hobo_check_failed()) {
      result.kind = HOBO_TEST_PASS;
      passed++;
    } else {
      result.kind = HOBO_TEST_FAIL;
      failed++;
    }
    result.skip_reason = NULL;
    result.checks = hobo_check_get();

    reporter->test_end(reporter->state, test, &result);
  }

  if (suite->suite_teardown != NULL) {
    suite->suite_teardown(ctx);
  }

  hobo_test_summary summary = {
      .total = count,
      .passed = passed,
      .failed = failed,
      .skipped = skipped,
  };
  reporter->suite_end(reporter->state, &summary);

  return failed != 0;
}
