#include "alloc.h"
#include <stdio.h>
#include <stdlib.h>

void *xmalloc(size_t size) {
  void *ptr = malloc(size);
  if (!ptr) {
    fprintf(stderr, "Out of memory: malloc(%zu)\n", size);
    exit(EXIT_FAILURE);
  }
  return ptr;
}

void *xrealloc(void *ptr, size_t size) {
  void *new_ptr = realloc(ptr, size);
  if (!new_ptr) {
    fprintf(stderr, "Out of memory: realloc(%zu)\n", size);
    exit(EXIT_FAILURE);
  }
  return new_ptr;
}