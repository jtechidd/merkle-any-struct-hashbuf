#ifndef buffer_h
#define buffer_h

#include <stdint.h>
#include <stdlib.h>

typedef struct buffer_t buffer_t;
typedef struct hash_data_t hash_data_t;

buffer_t *buffer_new();
void buffer_memcpy(buffer_t *, void *, size_t);
void buffer_memcpy_from_hash_data(buffer_t *, hash_data_t *);
uint8_t *buffer_get_byte_array(buffer_t *);
size_t buffer_get_capacity(buffer_t *);
size_t buffer_get_length(buffer_t *);
void buffer_debug(buffer_t *);
void buffer_free(buffer_t *);

#endif