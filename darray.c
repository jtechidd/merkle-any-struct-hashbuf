#include "darray.h"

#include <stdio.h>

#define INITIAL_CAPACITY 8

struct darray_t {
  void **array;
  size_t capacity;
  size_t length;
};

int darray_new(darray_t **darray_out) {
  darray_t *darray = malloc(sizeof(darray_t));
  if (!darray) {
    return -1;
  }
  darray->capacity = INITIAL_CAPACITY;
  darray->length = 0;
  darray->array = calloc(darray->capacity, sizeof(void *));
  if (!darray->array) {
    free(darray);
    return -1;
  }
  *darray_out = darray;
  return 0;
}

int darray_get_index(void **entry_out, darray_t *darray, size_t index) {
  if (!(index >= 0 && index < darray->length)) {
    *entry_out = NULL;
    return -1;
  }
  *entry_out = darray->array[index];
  return 0;
}

int darray_add(darray_t *darray, void *entry) {
  if (darray->length >= darray->capacity) {
    darray->capacity <<= 1;
    void **realloc_array = realloc(darray->array, darray->capacity * sizeof(void *));
    if (!realloc_array) {
      perror("realloc()");
      return -1;
    }
    darray->array = realloc_array;
  }
  darray->array[darray->length] = entry;
  darray->length++;
  return 0;
}

int darray_get_length(size_t *length_out, darray_t *darray) {
  *length_out = darray->length;
  return 0;
}

int darray_get_capacity(size_t *capacity_out, darray_t *darray) {
  *capacity_out = darray->capacity;
  return 0;
}

int darray_shallow_free(darray_t *darray) {
  free(darray->array);
  free(darray);
  return 0;
}

int darray_free(darray_t *darray) {
  for (size_t i = 0; i < darray->length; i++) {
    free(darray->array[i]);
  }
  free(darray->array);
  free(darray);
  return 0;
}