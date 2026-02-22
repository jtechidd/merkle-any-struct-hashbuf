#ifndef darray_h
#define darray_h

#include <stdlib.h>

typedef struct darray_t darray_t;

darray_t *darray_new();
void darray_add(darray_t *, void *);
size_t darray_get_length(darray_t *);
size_t darray_get_capacity(darray_t *);
void *darray_get_index(darray_t *, size_t);
void darray_shallow_free(darray_t *);
void darray_free(darray_t *);

#endif