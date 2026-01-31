#ifndef hash_buffer_h
#define hash_buffer_h

#include <stdint.h>
#include <stdlib.h>

typedef struct buffer_t buffer_t;

typedef int (*hash_t)(buffer_t *, uint8_t **, unsigned int *);

typedef struct hash_buffer_t {
  hash_t hash;
} hash_buffer_t;

#endif