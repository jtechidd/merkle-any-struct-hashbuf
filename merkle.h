#ifndef merkle_h
#define merkle_h

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct merkle_t merkle_t;
typedef struct merkle_node_t merkle_node_t;
typedef struct hash_buffer_t hash_buffer_t;
typedef struct hash_data_t hash_data_t;
typedef struct serializable_t serializable_t;
typedef struct darray_t darray_t;

int merkle_new(merkle_t **, hash_buffer_t *);
int merkle_build(merkle_t *, darray_t *);
int merkle_get_root(merkle_node_t **, merkle_t *);
int merkle_get_total_nodes(size_t *, merkle_t *);
int merkle_get_total_leaves(size_t *, merkle_t *);
int merkle_get_proof(darray_t *, merkle_t *, size_t);
int merkle_verify(bool *, merkle_t *, darray_t *);
int merkle_debug(merkle_t *);
int merkle_free(merkle_t *);

int merkle_node_new(merkle_node_t **, size_t, size_t, merkle_node_t *, merkle_node_t *);

#endif