#ifndef darray_h
#define darray_h

#include <stdlib.h>

typedef struct darray_t darray_t;

int darray_new(darray_t **);
int darray_add(darray_t *, void *);
int darray_get_length(size_t *, darray_t *);
int darray_get_capacity(size_t *, darray_t *);
int darray_get_index(void **, darray_t *, size_t);
int darray_shallow_free(darray_t *);
int darray_free(darray_t *);

#endif