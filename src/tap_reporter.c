#include "hobo/tap_reporter.h"
#include "hobo/test.h"
#include <stddef.h>
#include <stdio.h>

typedef struct {
  FILE *out;
  size_t test_number;
} tap_state;

static tap_state state;

static void tap_suite_begin(void *st, const hobo_test_suite *suite,
                            size_t test_count) {
  tap_state *s = st;
  (void)suite;
  fprintf(s->out, "TAP version 13\n");
  fprintf(s->out, "1..%zu\n", test_count);
  s->test_number = 1;
}

static void tap_test_end(void *st, const hobo_test_case *test,
                         const hobo_test_result *result) {
  tap_state *s = st;
  size_t n = s->test_number++;

  switch (result->kind) {
  case HOBO_TEST_PASS:
    fprintf(s->out, "ok %zu - %s\n", n, test->name);
    break;
  case HOBO_TEST_SKIP:
    fprintf(s->out, "ok %zu - %s # SKIP %s\n", n, test->name,
            result->skip_reason);
    break;
  case HOBO_TEST_FAIL:
    fprintf(s->out, "not ok %zu - %s\n", n, test->name);
    for (size_t i = 0; i < result->checks->count; i++) {
      const hobo_check_record *r = &result->checks->records[i];
      if (!r->passed) {

        fprintf(s->out, "# %s - %s:%d\n", r->expr, r->file, r->line);
      }
    }
    break;
  }
}

static void tap_suite_end(void *st, const hobo_test_summary *summary) {
  (void)st;
  (void)summary;
}

hobo_reporter hobo_tap_reporter(FILE *out) {
  state.out = out;
  hobo_reporter reporter = {
      .state = &state,
      .suite_begin = tap_suite_begin,
      .test_end = tap_test_end,
      .suite_end = tap_suite_end,
  };
  return reporter;
}
