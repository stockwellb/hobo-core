#include <assert.h>
#include <hobo/arena.h>
#include <stdalign.h>
#include <stdio.h>

int main(void) {
  hobo_arena a;

  /* init should succeed and report true */
  assert(hobo_arena_init(&a, 1024));

  /* the struct should be set up correctly */
  assert(a.base != NULL);
  assert(a.capacity == 1024);
  assert(a.offset == 0);

  void *p = hobo_arena_alloc(&a, 64);
  assert(p != NULL);
  assert(a.offset == 64);
  assert((size_t)p % alignof(max_align_t) == 0);

  void *q = hobo_arena_alloc_aligned(&a, 10, 64);
  assert(q != NULL);
  assert((size_t)q % 64 == 0);

  assert(hobo_arena_alloc(&a, 100000) == NULL);

  printf("test_arena: init ok\n");
  return 0;
}
