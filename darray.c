#include "darray.h"
#include "alloc.h"

#include <stdio.h>

#define INITIAL_CAPACITY 8

struct darray_t {
  void **array;
  size_t capacity;
  size_t length;
};

darray_t *darray_new() {
  darray_t *darray = xmalloc(sizeof(darray_t));
  darray->capacity = INITIAL_CAPACITY;
  darray->length = 0;
  darray->array = xmalloc(darray->capacity * sizeof(void *));
  return darray;
}

void *darray_get_index(darray_t *darray, size_t index) {
  if (!(index >= 0 && index < darray->length)) {
    return NULL;
  }
  return darray->array[index];
}

void darray_add(darray_t *darray, void *entry) {
  if (darray->length >= darray->capacity) {
    darray->capacity <<= 1;
    void **realloc_array =
        xrealloc(darray->array, darray->capacity * sizeof(void *));
    darray->array = realloc_array;
  }
  darray->array[darray->length] = entry;
  darray->length++;
}

size_t darray_get_length(darray_t *darray) { return darray->length; }

size_t darray_get_capacity(darray_t *darray) { return darray->capacity; }

void darray_shallow_free(darray_t *darray) {
  if (!darray) {
    return;
  }
  if (darray->array != NULL) {
    free(darray->array);
    darray->array = NULL;
  }
  free(darray);
}

void darray_free(darray_t *darray) {
  if (!darray) {
    return;
  }
  for (size_t i = 0; i < darray->length; i++) {
    if (darray->array[i] != NULL) {
      free(darray->array[i]);
      darray->array[i] = NULL;
    }
  }
  if (darray->array != NULL) {
    free(darray->array);
    darray->array = NULL;
  }
  free(darray);
}