#ifndef HOBO_ALLOCATOR_H
#define HOBO_ALLOCATOR_H
#include <stddef.h>

typedef struct hobo_allocator {
  void *(*alloc)(struct hobo_allocator *self, size_t size);
  void *(*realloc)(struct hobo_allocator *self, void *ptr, size_t old_size,
                   size_t new_size);
  void (*free)(struct hobo_allocator *self, void *ptr, size_t size);
} hobo_allocator;

#endif
