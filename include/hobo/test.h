#include <stdlib.h>

typedef enum { TEST_PASS, TEST_FAIL } hobo_test_result;

typedef struct {
  const char *name;
  void (*setup)(void *ctx);
  hobo_test_result (*run)(void *ctx);
  void (*teardown)(void *ctx);
  int skip;

} hobo_test_case;

typedef struct {
  const char *name;
  void *(*suite_setup)(void);
  hobo_test_case *tests;
  void (*suite_teardown)(void *ctx);
} hobo_test_suite;

int hobo_test_run_suite(hobo_test_suite *suite);
void hobo_test_run_all(void);
