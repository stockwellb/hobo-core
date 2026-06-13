#include "hobo/check.h"

int _hobo_check_failed = 0;

static void test_pass(const char *msg) { printf("  [PASS] %s\n", msg); }

static void test_fail(const char *msg, const char *file, int line) {
  _hobo_check_failed = 1;
  printf("  [FAIL] %s\n", msg);
  printf("         at %s:%d\n", file, line);
}

void test_check(bool passed, const char *expr, const char *file, int line) {
  if (passed) {
    test_pass(expr);
  } else {
    test_fail(expr, file, line);
  }
}

void test_check_eq_int(int actual, int expected, const char *actual_expr,
                       const char *expected_expr, const char *file, int line) {
  if (actual == expected) {
    test_pass("ok");
  } else {
    test_fail("integer equality failed", file, line);
    printf("         actual:   %s = %d\n", actual_expr, actual);
    printf("         expected: %s = %d\n", expected_expr, expected);
  }
}

void test_check_ne_int(int actual, int expected, const char *actual_expr,
                       const char *expected_expr, const char *file, int line) {
  if (actual != expected) {
    test_pass("ok");
  } else {
    test_fail("integer inequality failed", file, line);
    printf("         actual:   %s = %d\n", actual_expr, actual);
    printf("         expected: %s = %d\n", expected_expr, expected);
  }
}

void test_check_not_null(const void *ptr, const char *ptr_expr,
                         const char *file, int line) {
  if (ptr != NULL) {
    test_pass(ptr_expr);
  } else {
    test_fail("expected non-null pointer", file, line);
    printf("         actual:   %s = NULL\n", ptr_expr);
  }
}

void test_check_null(const void *ptr, const char *ptr_expr, const char *file,
                     int line) {
  if (ptr == NULL) {
    test_pass(ptr_expr);
  } else {
    test_fail("expected null pointer", file, line);
    printf("         actual:   %s = %p\n", ptr_expr, ptr);
  }
}
