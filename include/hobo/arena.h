#ifndef HOBO_ARENA_H
#define HOBO_ARENA_H

#include <stdbool.h>
#include <stddef.h>

typedef struct hobo_arena {
  unsigned char *base;
  size_t offset;
  size_t capacity;
} hobo_arena;

bool hobo_arena_init(hobo_arena *arena, size_t capacity);

#endif
