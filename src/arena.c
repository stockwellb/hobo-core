#include "hobo/allocator.h"
#include <hobo/arena.h>
#include <stdalign.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

static void *arena_alloc(hobo_allocator *self, size_t size) {
  hobo_arena *arena = (hobo_arena *)self;
  return hobo_arena_alloc(arena, size);
}

static void *arena_realloc(hobo_allocator *self, void *ptr, size_t old_size,
                           size_t new_size) {
  void *new_ptr = arena_alloc(self, new_size);
  size_t mem_size = old_size < new_size ? old_size : new_size;
  if (new_ptr && ptr) {
    memcpy(new_ptr, ptr, mem_size);
  }
  return new_ptr;
}

static void arena_free(hobo_allocator *self, void *ptr, size_t size) {
  (void)self;
  (void)ptr;
  (void)size; // no-op for arena
}

bool hobo_arena_init(hobo_arena *arena, size_t capacity) {
  arena->base.alloc = arena_alloc;
  arena->base.realloc = arena_realloc;
  arena->base.free = arena_free;

  if (capacity == 0) {
    arena->buf = NULL;
    return false;
  }

  arena->buf = malloc(capacity);

  if (arena->buf == NULL) {
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

  void *ptr = arena->buf + aligned;
  arena->offset = aligned + size;

  return ptr;
}

void *hobo_arena_alloc(hobo_arena *arena, size_t size) {
  return hobo_arena_alloc_aligned(arena, size, alignof(max_align_t));
}
