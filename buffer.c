#include "buffer.h"

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

int buffer_new(buffer_t **buffer_out) {
  buffer_t *buffer = malloc(sizeof(buffer_t));
  if (!buffer) {
    return -1;
  }
  buffer->capacity = INITIAL_CAPACITY;
  buffer->length = 0;
  buffer->byte_array = malloc(buffer->capacity * sizeof(uint8_t));
  if (!buffer->byte_array) {
    free(buffer);
    return -1;
  }
  *buffer_out = buffer;
  return 0;
}

int buffer_memcpy(buffer_t *buffer, void *data, size_t size) {
  size_t capacity = buffer->capacity;
  while (buffer->length + size > capacity) {
    capacity <<= 1;
  }
  if (capacity > buffer->capacity) {
    uint8_t *new_byte_array = realloc(buffer->byte_array, capacity * sizeof(uint8_t));
    if (!new_byte_array) {
      perror("realloc()");
      return -1;
    }
    buffer->byte_array = new_byte_array;
    buffer->capacity = capacity;
  }
  memcpy(buffer->byte_array + buffer->length, data, size);
  buffer->length += size;
  return 0;
}

int buffer_get_byte_array(uint8_t **byte_array_out, buffer_t *buffer) {
  *byte_array_out = buffer->byte_array;
  return 0;
}

int buffer_get_capacity(size_t *capacity_out, buffer_t *buffer) {
  *capacity_out = buffer->capacity;
  return 0;
}

int buffer_get_length(size_t *length_out, buffer_t *buffer) {
  *length_out = buffer->length;
  return 0;
}

int buffer_debug(buffer_t *buffer) {
  for (size_t i = 0; i < buffer->length; i++) {
    printf("%02x", buffer->byte_array[i]);
  }
  printf("\n");
  return 0;
}

int buffer_free(buffer_t *buffer) {
  free(buffer->byte_array);
  free(buffer);
  return 0;
}