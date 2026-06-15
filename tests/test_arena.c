#include <hobo/arena.h>
#include <hobo/check.h>
#include <hobo/tap_reporter.h>
#include <hobo/test.h>
#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

typedef struct {
  hobo_arena arena;
} arena_ctx;

static void *arena_suite_setup(void) {
  arena_ctx *ctx = malloc(sizeof(*ctx));
  return ctx;
}

static void arena_suite_teardown(void *raw) {
  arena_ctx *ctx = raw;
  free(ctx);
}

static void test_setup(void *raw) {
  arena_ctx *ctx = raw;
  hobo_arena_init(&ctx->arena, 1024);
}

static void test_teardown(void *raw) {
  arena_ctx *ctx = raw;
  free(ctx->arena.base);
}

static bool test_arena_init(void *raw) {
  arena_ctx *ctx = raw;
  CHECK(ctx->arena.base != NULL);
  CHECK(ctx->arena.capacity == 1024);
  CHECK(ctx->arena.offset == 0);
  return HOBO_CHECK_RESULT();
}

static bool test_arena_alloc(void *raw) {
  arena_ctx *ctx = raw;
  void *p = hobo_arena_alloc(&ctx->arena, 64);
  CHECK(p != NULL);
  CHECK(ctx->arena.offset == 64);
  CHECK((uintptr_t)p % alignof(max_align_t) == 0);
  return HOBO_CHECK_RESULT();
}

static bool test_arena_alloc_aligned(void *raw) {
  arena_ctx *ctx = raw;
  (void)hobo_arena_alloc_aligned(&ctx->arena, 10, 64);
  void *p = hobo_arena_alloc_aligned(&ctx->arena, 10, 64);
  CHECK(p != NULL);
  CHECK(ctx->arena.offset == 64 + 10);
  CHECK((uintptr_t)p % 64 == 0);
  return HOBO_CHECK_RESULT();
}

static bool test_arena_alloc_overflow(void *raw) {
  arena_ctx *ctx = raw;
  void *p = hobo_arena_alloc(&ctx->arena, 100000);
  CHECK(p == NULL);
  return HOBO_CHECK_RESULT();
}

static bool test_arena_seq_alloc(void *raw) {
  arena_ctx *ctx = raw;
  void *p1 = hobo_arena_alloc(&ctx->arena, 2);
  CHECK(p1 != NULL);
  CHECK(ctx->arena.offset == 2);
  void *p2 = hobo_arena_alloc(&ctx->arena, 2);
  size_t a = alignof(max_align_t);
  size_t expected = ((2 + a - 1) & ~(a - 1)) + 2;
  CHECK(p2 != NULL);
  CHECK(ctx->arena.offset == expected);
  CHECK(p1 != p2);
  return HOBO_CHECK_RESULT();
}

static bool test_arena_full_alloc(void *raw) {
  arena_ctx *ctx = raw;
  void *p1 = hobo_arena_alloc(&ctx->arena, 1024);
  CHECK(p1 != NULL);
  CHECK(ctx->arena.offset == 1024);
  return HOBO_CHECK_RESULT();
}

static bool test_arena_alloc_overflow_by_one(void *raw) {
  arena_ctx *ctx = raw;
  void *p = hobo_arena_alloc(&ctx->arena, 1025);
  CHECK(p == NULL);
  return HOBO_CHECK_RESULT();
}

static bool test_arena_alloc_zero(void *raw) {
  arena_ctx *ctx = raw;
  void *p = hobo_arena_alloc(&ctx->arena, 0);
  CHECK(p == NULL);
  return HOBO_CHECK_RESULT();
}

static bool test_arena_zero_init(void *raw) {
  (void)raw;
  hobo_arena zero_arena;
  hobo_arena_init(&zero_arena, 0);
  CHECK(zero_arena.base == NULL);
  return HOBO_CHECK_RESULT();
}

static hobo_test_case arena_tests[] = {
    {"test_arena_init", test_setup, test_arena_init, test_teardown, 0},
    {"test_arena_alloc", test_setup, test_arena_alloc, test_teardown, 0},
    {"test_arena_alloc_aligned", test_setup, test_arena_alloc_aligned,
     test_teardown, 0},
    {"test_arena_alloc_overflow", test_setup, test_arena_alloc_overflow,
     test_teardown, 0},
    {"test_arena_seq_alloc", test_setup, test_arena_seq_alloc, test_teardown,
     0},
    {"test_arena_full_alloc", test_setup, test_arena_full_alloc, test_teardown,
     0},
    {"test_arena_alloc_overflow_by_one", test_setup,
     test_arena_alloc_overflow_by_one, test_teardown, 0},
    {"test_arena_alloc_zero", test_setup, test_arena_alloc_zero, test_teardown,
     0},
    {"test_arena_zero_init", NULL, test_arena_zero_init, NULL, 0},
    {0}};

static hobo_test_suite arena_suite = {
    .name = "arena",
    .suite_setup = arena_suite_setup,
    .suite_teardown = arena_suite_teardown,
    .tests = arena_tests,
};

int main(void) {
  hobo_reporter reporter = hobo_tap_reporter(stdout);
  return hobo_test_run_suite(&arena_suite, &reporter);
}
