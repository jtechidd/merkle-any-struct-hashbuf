#include "buffer.h"
#include "alloc.h"
#include "hash_data.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 8

struct buffer_t {
  uint8_t *byte_array;
  size_t capacity;
  size_t length;
};

buffer_t *buffer_new() {
  buffer_t *buffer = xmalloc(sizeof(buffer_t));
  buffer->capacity = INITIAL_CAPACITY;
  buffer->length = 0;
  buffer->byte_array = xmalloc(buffer->capacity * sizeof(uint8_t));
  return buffer;
}

void buffer_memcpy(buffer_t *buffer, void *data, size_t size) {
  size_t capacity = buffer->capacity;
  while (buffer->length + size > capacity) {
    capacity <<= 1;
  }
  if (capacity > buffer->capacity) {
    uint8_t *new_byte_array =
        xrealloc(buffer->byte_array, capacity * sizeof(uint8_t));
    buffer->byte_array = new_byte_array;
    buffer->capacity = capacity;
  }
  memcpy(buffer->byte_array + buffer->length, data, size);
  buffer->length += size;
}

void buffer_memcpy_from_hash_data(buffer_t *buffer, hash_data_t *hash_data) {
  uint8_t *hash = hash_data_get_hash(hash_data);
  uint32_t hash_length = hash_data_get_hash_length(hash_data);
  buffer_memcpy(buffer, hash, hash_length);
}

uint8_t *buffer_get_byte_array(buffer_t *buffer) { return buffer->byte_array; }

size_t buffer_get_capacity(buffer_t *buffer) {
  return buffer->capacity;
}

size_t buffer_get_length(buffer_t *buffer) {
  return buffer->length;
}

void buffer_debug(buffer_t *buffer) {
  for (size_t i = 0; i < buffer->length; i++) {
    printf("%02x", buffer->byte_array[i]);
  }
  printf("\n");
}

void buffer_free(buffer_t *buffer) {
  if(!buffer) {
    return;
  }
  if (buffer->byte_array != NULL) {
    free(buffer->byte_array);
    buffer->byte_array = NULL;
  }
  free(buffer);
}