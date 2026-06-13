#ifndef HOBO_CHECK_H
#define HOBO_CHECK_H
#include <stdio.h>

extern int _hobo_check_failed;

#define HOBO_CHECK_RESET() (_hobo_check_failed = 0)
#define HOBO_CHECK_RESULT() (_hobo_check_failed ? TEST_FAIL : TEST_PASS)

#define CHECK(cond)                                                            \
  do {                                                                         \
    if (!(cond)) {                                                             \
      _hobo_check_failed = 1;                                                  \
      printf("  [FAIL] %s\n", #cond);                                          \
      printf("         at %s:%d\n", __FILE__, __LINE__);                       \
    } else {                                                                   \
      printf("  [PASS] %s\n", #cond);                                          \
    }                                                                          \
  } while (0)

#endif
