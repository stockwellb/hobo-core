#ifndef HOBO_CHECK_H
#define HOBO_CHECK_H
#include <stdbool.h>
#include <stdio.h>

extern int _hobo_check_failed;

void test_check(bool passed, const char *expr, const char *file, int line);
void test_check_eq_int(int actual, int expected, const char *actual_expr,
                       const char *expected_expr, const char *file, int line);

void test_check_ne_int(int actual, int expected, const char *actual_expr,
                       const char *expected_expr, const char *file, int line);

void test_check_eq_float(float actual, float expected, float epsilon,
                         const char *actual_expr, const char *expected_expr,
                         const char *file, int line);

void test_check_null(const void *ptr, const char *ptr_expr, const char *file,
                     int line);

void test_check_not_null(const void *ptr, const char *ptr_expr,
                         const char *file, int line);

#define HOBO_CHECK_RESET() (_hobo_check_failed = 0)
#define HOBO_CHECK_RESULT() (_hobo_check_failed ? TEST_FAIL : TEST_PASS)
#define CHECK(cond) test_check(!!(cond), #cond, __FILE__, __LINE__)
#define CHECK_TRUE(cond)                                                       \
  test_check(!!(cond), #cond " is true", __FILE__, __LINE__)

#define CHECK_FALSE(cond)                                                      \
  test_check(!(cond), #cond " is false", __FILE__, __LINE__)

#define CHECK_EQ_INT(actual, expected)                                         \
  test_check_eq_int((actual), (expected), #actual, #expected, __FILE__,        \
                    __LINE__)

#define CHECK_NE_INT(actual, expected)                                         \
  test_check_ne_int((actual), (expected), #actual, #expected, __FILE__,        \
                    __LINE__)

#define CHECK_NULL(ptr) test_check_null((ptr), #ptr, __FILE__, __LINE__)
#define CHECK_NOT_NULL(ptr) test_check_not_null((ptr), #ptr, __FILE__, __LINE__)

#endif
