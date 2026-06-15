#include <hobo/arena.h>
#include <stdalign.h>
#include <stdlib.h>

bool hobo_arena_init(hobo_arena *arena, size_t capacity) {
  if (capacity == 0) {
    arena->base = NULL;
    return false;
  }

  arena->base = malloc(capacity);

  if (arena->base == NULL) {
    return false;
  }

  arena->capacity = capacity;
  arena->offset = 0;

  return true;
}

void *hobo_arena_alloc_aligned(hobo_arena *arena, size_t size, size_t align) {
  size_t aligned = (arena->offset + align - 1) & ~(align - 1);

  if (size == 0) {
    return NULL;
  }

  if (aligned > arena->capacity || size > arena->capacity - aligned) {
    return NULL;
  }

  void *ptr = arena->base + aligned;
  arena->offset = aligned + size;

  return ptr;
}

void *hobo_arena_alloc(hobo_arena *arena, size_t size) {
  return hobo_arena_alloc_aligned(arena, size, alignof(max_align_t));
}
