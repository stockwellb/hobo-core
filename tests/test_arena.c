#include <assert.h>
#include <hobo/arena.h>
#include <stdio.h>

int main(void) {
  hobo_arena a;

  /* init should succeed and report true */
  assert(hobo_arena_init(&a, 1024));

  /* the struct should be set up correctly */
  assert(a.base != NULL);
  assert(a.capacity == 1024);
  assert(a.offset == 0);

  printf("test_arena: init ok\n");
  return 0;
}
