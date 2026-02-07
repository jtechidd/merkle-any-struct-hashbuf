#ifndef hash_buffer_h
#define hash_buffer_h

#include <stdint.h>
#include <stdlib.h>

typedef struct buffer_t buffer_t;
typedef struct hash_data_t hash_data_t;

typedef int hash_t(hash_data_t *, buffer_t *);

typedef struct hash_buffer_t {
  hash_t *hash;
} hash_buffer_t;

#endif