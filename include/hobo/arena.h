#ifndef HOBO_ARENA_H
#define HOBO_ARENA_H

#include <hobo/allocator.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct hobo_arena {
  hobo_allocator base;
  unsigned char *buf;
  size_t offset;
  size_t capacity;
} hobo_arena;

bool hobo_arena_init(hobo_arena *arena, size_t capacity);

void *hobo_arena_alloc(hobo_arena *arena, size_t size);

void *hobo_arena_alloc_aligned(hobo_arena *arena, size_t size, size_t align);

#endif
