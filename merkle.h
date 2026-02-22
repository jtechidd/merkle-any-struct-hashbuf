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

merkle_t *merkle_new(hash_buffer_t *);
void merkle_build(merkle_t *, darray_t *);
merkle_node_t *merkle_get_root(merkle_t *);
size_t merkle_get_total_nodes(merkle_t *);
size_t merkle_get_total_leaves(merkle_t *);
darray_t *merkle_get_proof(merkle_t *, size_t);
bool merkle_verify(merkle_t *, darray_t *);
void merkle_debug(merkle_t *);
void merkle_free(merkle_t *);

merkle_node_t *merkle_node_new(size_t, size_t, merkle_node_t *, merkle_node_t *,
                               hash_data_t *);
void merkle_node_free(merkle_node_t *);

#endif