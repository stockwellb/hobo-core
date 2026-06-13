#include <hobo/arena.h>
#include <stdlib.h>

bool hobo_arena_init(hobo_arena *arena, size_t capacity) {
  arena->base = malloc(capacity);

  if (arena->base == NULL) {
    return false;
  }

  arena->capacity = capacity;
  arena->offset = 0;

  return true;
}
