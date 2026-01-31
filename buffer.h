#ifndef buffer_h
#define buffer_h

#include <stdint.h>
#include <stdlib.h>

typedef struct buffer_t buffer_t;

int buffer_new(buffer_t **);
int buffer_memcpy(buffer_t *, void *, size_t);
int buffer_get_byte_array(buffer_t *, uint8_t **);
int buffer_get_capacity(buffer_t *, size_t *);
int buffer_get_length(buffer_t *, size_t *);
int buffer_free(buffer_t *);

#endif