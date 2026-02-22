#include "merkle.h"
#include "alloc.h"
#include "buffer.h"
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
  hash_data_t *hash_data;
};

merkle_node_t *merkle_node_new(size_t range_start, size_t range_end,
                               merkle_node_t *node_left,
                               merkle_node_t *node_right,
                               hash_data_t *hash_data) {
  merkle_node_t *merkle_node = (merkle_node_t *)xmalloc(sizeof(merkle_node_t));
  merkle_node->range_start = range_start;
  merkle_node->range_end = range_end;
  merkle_node->node_left = node_left;
  merkle_node->node_right = node_right;
  merkle_node->hash_data = hash_data;
  return merkle_node;
}

void merkle_node_free(merkle_node_t *merkle_node) {
  if (!merkle_node) {
    return;
  }
  if (merkle_node->hash_data != NULL) {
    free(merkle_node->hash_data);
    merkle_node->hash_data = NULL;
  }
  free(merkle_node);
}

void merkle_node_recursively_free(merkle_node_t *merkle_node) {
  if (merkle_node->node_left != NULL) {
    merkle_node_recursively_free(merkle_node->node_left);
    merkle_node->node_left = NULL;
  }
  if (merkle_node->node_right != NULL) {
    merkle_node_recursively_free(merkle_node->node_right);
    merkle_node->node_right = NULL;
  }
  merkle_node_free(merkle_node);
}

merkle_t *merkle_new(hash_buffer_t *hash_buffer) {
  merkle_t *merkle = xmalloc(sizeof(merkle_t));
  merkle->root = NULL;
  merkle->hash_buffer = hash_buffer;
  merkle->total_leaves = 0;
  merkle->total_nodes = 0;
  return merkle;
}

void sort_hash_data(hash_data_t **hash_data_left,
                    hash_data_t **hash_data_right) {
  int cmp_result = hash_data_cmp(*hash_data_left, *hash_data_right);
  if (cmp_result > 0) {
    hash_data_t *tmp = *hash_data_left;
    *hash_data_left = *hash_data_right;
    *hash_data_right = tmp;
  }
}

merkle_node_t *merkle_get_root(merkle_t *merkle) { return merkle->root; }

size_t merkle_get_total_nodes(merkle_t *merkle) { return merkle->total_nodes; }

size_t merkle_get_total_leaves(merkle_t *merkle) {
  return merkle->total_leaves;
}

static merkle_node_t *merkle_build_recursive(merkle_t *merkle, darray_t *darray,
                                             size_t range_start,
                                             size_t range_end) {
  if (range_start == range_end) {
    serializable_t *serializable = darray_get_index(darray, range_start);
    buffer_t *buffer = buffer_new();

    serializable->serialize(serializable, buffer);

    hash_data_t *hash_data = hash_data_new();
    merkle->hash_buffer->hash(hash_data, buffer);

    merkle_node_t *merkle_node =
        merkle_node_new(range_start, range_end, NULL, NULL, hash_data);

    merkle->total_nodes++;
    merkle->total_leaves++;
    return merkle_node;
  }

  size_t mid = (range_start + range_end) >> 1;
  merkle_node_t *node_left =
      merkle_build_recursive(merkle, darray, range_start, mid);
  merkle_node_t *node_right =
      merkle_build_recursive(merkle, darray, mid + 1, range_end);

  hash_data_t *node_left_hash_data = node_left->hash_data;
  hash_data_t *node_right_hash_data = node_right->hash_data;
  sort_hash_data(&node_left_hash_data, &node_right_hash_data);

  buffer_t *buffer = buffer_new();
  buffer_memcpy_from_hash_data(buffer, node_left_hash_data);
  buffer_memcpy_from_hash_data(buffer, node_right_hash_data);

  hash_data_t *hash_data = hash_data_new();
  merkle->hash_buffer->hash(hash_data, buffer);

  merkle_node_t *node_parent =
      merkle_node_new(range_start, range_end, node_left, node_right, hash_data);

  merkle->total_nodes++;
  return node_parent;
}

void merkle_build(merkle_t *merkle, darray_t *darray) {
  int ret;
  size_t darray_length = darray_get_length(darray);
  merkle->root = merkle_build_recursive(merkle, darray, 0, darray_length - 1);
}

void merkle_get_proof_recursive(darray_t *proof_array_out, merkle_t *merkle,
                                merkle_node_t *node, size_t index,
                                size_t range_start, size_t range_end) {
  int ret;
  if (range_start == range_end) {
    darray_add(proof_array_out, node->hash_data);
    return;
  }
  size_t mid = (range_start + range_end) >> 1;
  if (index <= mid) {
    merkle_get_proof_recursive(proof_array_out, merkle, node->node_left, index,
                               range_start, mid);
    darray_add(proof_array_out, node->node_right->hash_data);
  } else {
    merkle_get_proof_recursive(proof_array_out, merkle, node->node_right, index,
                               mid + 1, range_end);
    darray_add(proof_array_out, node->node_left->hash_data);
  }
}

darray_t *merkle_get_proof(merkle_t *merkle, size_t index) {
  int ret;
  if (!(index >= 0 && index < merkle->total_leaves)) {
    return NULL;
  }
  darray_t *proof_array = darray_new();
  merkle_get_proof_recursive(proof_array, merkle, merkle->root, index, 0,
                             merkle->total_leaves - 1);
  return proof_array;
}

bool merkle_verify(merkle_t *merkle, darray_t *proof_array) {
  size_t proof_array_length = darray_get_length(proof_array);

  hash_data_t *first_proof_hash_data = NULL;
  hash_data_t *accumulated_hash_data = NULL;

  first_proof_hash_data = darray_get_index(proof_array, 0);
  accumulated_hash_data = hash_data_clone(first_proof_hash_data);

  for (size_t i = 1; i < proof_array_length; i++) {
    hash_data_t* hash_data = darray_get_index(proof_array, i);

    hash_data_t *left_hash_data = accumulated_hash_data;
    hash_data_t *right_hash_data = hash_data;

    sort_hash_data(&left_hash_data, &right_hash_data);

    buffer_t *buffer = buffer_new(&buffer);
    buffer_memcpy_from_hash_data(buffer, left_hash_data);
    buffer_memcpy_from_hash_data(buffer, right_hash_data);

    hash_data_t *new_accumulated_hash_data = hash_data_new();
    merkle->hash_buffer->hash(new_accumulated_hash_data, buffer);

    buffer_free(buffer);
    hash_data_free(accumulated_hash_data);

    accumulated_hash_data = new_accumulated_hash_data;
  }

  int cmp_result =
      hash_data_cmp(accumulated_hash_data, merkle->root->hash_data);

  hash_data_free(accumulated_hash_data);
  return !cmp_result;
}

void merkle_walk(merkle_t *merkle, merkle_node_t *merkle_node) {
  printf("Range: [%ld, %ld]\n", merkle_node->range_start,
         merkle_node->range_end);
  uint8_t *hash = hash_data_get_hash(merkle_node->hash_data);
  uint32_t hash_length = hash_data_get_hash_length(merkle_node->hash_data);
  for (size_t i = 0; i < hash_length; i++) {
    printf("%02x", hash[i]);
  }
  printf("\nNode left address: %p\n", merkle_node->node_left);
  printf("Node right address: %p\n\n", merkle_node->node_right);

  if (merkle_node->node_left) {
    merkle_walk(merkle, merkle_node->node_left);
  }
  if (merkle_node->node_right) {
    merkle_walk(merkle, merkle_node->node_right);
  }
}

void merkle_debug(merkle_t *merkle) {
  printf("Total nodes: %ld\n", merkle->total_nodes);
  printf("Total leaves: %ld\n\n", merkle->total_leaves);

  merkle_walk(merkle, merkle->root);
}