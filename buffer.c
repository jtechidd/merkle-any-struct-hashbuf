#include "buffer.h"

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

int buffer_new(buffer_t **buffer_out) {
  buffer_t *buffer = malloc(sizeof(buffer_t));
  if (!buffer) {
    *buffer_out = NULL;
    return -1;
  }
  buffer->capacity = INITIAL_CAPACITY;
  buffer->length = 0;
  buffer->byte_array = malloc(buffer->capacity);
  if (!buffer->byte_array) {
    free(buffer);
    *buffer_out = NULL;
    return -1;
  }
  *buffer_out = buffer;
  return 0;
}

int buffer_memcpy(buffer_t *buffer, void *data, size_t size) {
  while (buffer->length + size > buffer->capacity) {
    buffer->capacity <<= 1;
    uint8_t *new_byte_array = realloc(buffer->byte_array, buffer->capacity);
    if (!new_byte_array) {
      perror("realloc()");
      return -1;
    }
    buffer->byte_array = new_byte_array;
  }
  memcpy(buffer->byte_array + buffer->length, data, size);
  buffer->length += size;
  return 0;
}

int buffer_get_byte_array(buffer_t *buffer, uint8_t **byte_array_out) {
  *byte_array_out = buffer->byte_array;
  return 0;
}

int buffer_get_capacity(buffer_t *buffer, size_t *capacity_out) {
  *capacity_out = buffer->capacity;
  return 0;
}

int buffer_get_length(buffer_t *buffer, size_t *length_out) {
  *length_out = buffer->length;
  return 0;
}

int buffer_free(buffer_t *buffer) {
  free(buffer->byte_array);
  free(buffer);
  return 0;
}