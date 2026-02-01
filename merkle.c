#include "merkle.h"
#include "buffer.h"
#include "common.h"
#include "darray.h"
#include "hash_buffer.h"
#include "hash_data.h"
#include "serializable.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct merkle_t {
  merkle_node_t *root;
  hash_buffer_t *hash_buffer;
  size_t total_leaves;
  size_t total_nodes;
};

struct merkle_node_t {
  size_t range_start;
  size_t range_end;
  merkle_node_t *node_left;
  merkle_node_t *node_right;
  hash_data_t hash_data;
};

int merkle_node_new(merkle_node_t **merkle_node_out, size_t range_start, size_t range_end, merkle_node_t *node_left, merkle_node_t *node_right) {
  merkle_node_t *merkle_node = malloc(sizeof(merkle_node_t));
  if (!merkle_node) {
    return -1;
  }
  merkle_node->range_start = range_start;
  merkle_node->range_end = range_end;
  merkle_node->node_left = node_left;
  merkle_node->node_right = node_right;
  *merkle_node_out = merkle_node;
  return 0;
}

int merkle_new(merkle_t **merkle_out, hash_buffer_t *hash_buffer) {
  merkle_t *merkle = malloc(sizeof(merkle_t));
  if (!merkle) {
    *merkle_out = NULL;
    return -1;
  }
  merkle->root = NULL;
  merkle->hash_buffer = hash_buffer;
  merkle->total_leaves = 0;
  merkle->total_nodes = 0;
  *merkle_out = merkle;
  return 0;
}

int sort_hash_data(hash_data_t **hash_data_left, hash_data_t **hash_data_right) {
  if (strcmp((const char *)((*hash_data_left)->hash), (const char *)((*hash_data_right)->hash)) > 0) {
    hash_data_t *tmp = *hash_data_left;
    *hash_data_left = *hash_data_right;
    *hash_data_right = tmp;
  }
  return 0;
}

int merkle_get_root(merkle_node_t **root_node_out, merkle_t *merkle) {
  *root_node_out = merkle->root;
  return 0;
}

int merkle_get_total_nodes(size_t *total_leaves_out, merkle_t *merkle) {
  *total_leaves_out = merkle->total_nodes;
  return 0;
}

int merkle_get_total_leaves(size_t *total_leaves_out, merkle_t *merkle) {
  *total_leaves_out = merkle->total_leaves;
  return 0;
}

int merkle_build_recursive(merkle_node_t **merkle_node_out, merkle_t *merkle, darray_t *darray, size_t range_start, size_t range_end) {
  if (range_start == range_end) {
    merkle_node_t *merkle_node;
    TRY(merkle_node_new(&merkle_node, range_start, range_end, NULL, NULL))

    serializable_t *serializable;
    TRY(darray_get_index((void **)&serializable, darray, range_start))

    buffer_t *buffer;
    TRY(buffer_new(&buffer))

    TRY(serializable->serialize(serializable, buffer))

    TRY(merkle->hash_buffer->hash(&merkle_node->hash_data, buffer))

    *merkle_node_out = merkle_node;
    merkle->total_nodes++;
    merkle->total_leaves++;
    TRY(buffer_free(buffer))
    return 0;
  }

  size_t mid = (range_start + range_end) >> 1;

  merkle_node_t *node_left = NULL;
  TRY(merkle_build_recursive(&node_left, merkle, darray, range_start, mid))
  merkle_node_t *node_right = NULL;
  TRY(merkle_build_recursive(&node_right, merkle, darray, mid + 1, range_end))

  merkle_node_t *node_parent = NULL;
  TRY(merkle_node_new(&node_parent, range_start, range_end, node_left, node_right))

  buffer_t *buffer;
  TRY(buffer_new(&buffer))

  hash_data_t *node_left_hash_data = &node_left->hash_data;
  hash_data_t *node_right_hash_data = &node_right->hash_data;
  sort_hash_data(&node_left_hash_data, &node_right_hash_data);
  TRY(buffer_memcpy(buffer, node_left_hash_data->hash, node_left_hash_data->hash_length))
  TRY(buffer_memcpy(buffer, node_right_hash_data->hash, node_right_hash_data->hash_length))

  TRY(merkle->hash_buffer->hash(&node_parent->hash_data, buffer))

  *merkle_node_out = node_parent;
  merkle->total_nodes++;
  TRY(buffer_free(buffer))
  return 0;
}

int merkle_build(merkle_t *merkle, darray_t *darray) {
  size_t darray_length;
  TRY(darray_get_length(&darray_length, darray));
  TRY(merkle_build_recursive(&merkle->root, merkle, darray, 0, darray_length - 1))
  return 0;
}

int merkle_get_proof_recursive(darray_t *proof_array_out, merkle_t *merkle, merkle_node_t *node, size_t index, size_t range_start, size_t range_end) {
  if (range_start == range_end) {
    TRY(darray_add(proof_array_out, &node->hash_data))
    return 0;
  }
  size_t mid = (range_start + range_end) >> 1;
  if (index <= mid) {
    TRY(merkle_get_proof_recursive(proof_array_out, merkle, node->node_left, index, range_start, mid))
    TRY(darray_add(proof_array_out, &node->node_right->hash_data))
  } else {
    TRY(merkle_get_proof_recursive(proof_array_out, merkle, node->node_right, index, mid + 1, range_end))
    TRY(darray_add(proof_array_out, &node->node_left->hash_data))
  }
  return 0;
}

int merkle_get_proof(darray_t *proof_array_out, merkle_t *merkle, size_t index) {
  if (!(index >= 0 && index < merkle->total_leaves)) {
    return -1;
  }
  TRY(merkle_get_proof_recursive(proof_array_out, merkle, merkle->root, index, 0, merkle->total_leaves - 1))
  return 0;
}

int merkle_verify(bool *verify_ok_out, merkle_t *merkle, darray_t *proof_array) {
  size_t proof_array_length;
  TRY(darray_get_length(&proof_array_length, proof_array))

  if (proof_array_length == 1) {
    hash_data_t *proof_hash_data;
    TRY(darray_get_index((void **)&proof_hash_data, proof_array, 0))
    *verify_ok_out = !strcmp((const char *)proof_hash_data->hash, (const char *)merkle->root->hash_data.hash);
    return 0;
  }

  hash_data_t *accumulated_hash_data;
  hash_data_t *first_proof_hash_data;
  TRY(darray_get_index((void **)&first_proof_hash_data, proof_array, 0))
  TRY(hash_data_clone(&accumulated_hash_data, first_proof_hash_data))

  for (size_t i = 1; i < proof_array_length; i++) {
    hash_data_t *hash_data;
    TRY(darray_get_index((void **)&hash_data, proof_array, i))

    hash_data_t *hash_data_left = accumulated_hash_data;
    hash_data_t *hash_data_right = hash_data;
    TRY(sort_hash_data(&hash_data_left, &hash_data_right))

    buffer_t *buffer;
    TRY(buffer_new(&buffer))

    TRY(buffer_memcpy(buffer, hash_data_left->hash, hash_data_left->hash_length))
    TRY(buffer_memcpy(buffer, hash_data_right->hash, hash_data_right->hash_length))

    hash_data_t *new_accumulated_hash_data;
    TRY(hash_data_new(&new_accumulated_hash_data))
    TRY(merkle->hash_buffer->hash(new_accumulated_hash_data, buffer))

    TRY(buffer_free(buffer))
    TRY(hash_data_free(accumulated_hash_data))
    accumulated_hash_data = new_accumulated_hash_data;
  }

  *verify_ok_out = !strcmp((const char *)accumulated_hash_data->hash, (const char *)merkle->root->hash_data.hash);
  return 0;
}

int merkle_walk(merkle_t *merkle, merkle_node_t *merkle_node) {
  printf("Range: [%ld, %ld]\n", merkle_node->range_start, merkle_node->range_end);
  printf("Hash: ");
  for (size_t i = 0; i < merkle_node->hash_data.hash_length; i++) {
    printf("%02x", merkle_node->hash_data.hash[i]);
  }
  printf("\nNode left address: %p\n", merkle_node->node_left);
  printf("Node right address: %p\n\n", merkle_node->node_right);

  if (merkle_node->node_left) {
    TRY(merkle_walk(merkle, merkle_node->node_left))
  }
  if (merkle_node->node_right) {
    TRY(merkle_walk(merkle, merkle_node->node_right))
  }
  return 0;
}

int merkle_debug(merkle_t *merkle) {
  printf("Total nodes: %ld\n", merkle->total_nodes);
  printf("Total leaves: %ld\n\n", merkle->total_leaves);

  TRY(merkle_walk(merkle, merkle->root))
  return 0;
}