#include "merkle.h"
#include "buffer.h"
#include "common.h"
#include "hash_buffer.h"
#include "serializable.h"

#include <stdio.h>
#include <stdlib.h>

struct merkle_t {
  merkle_node_t *root;
  hash_buffer_t *hash_buffer;
};

struct merkle_node_t {
  size_t range_start;
  size_t range_end;
  merkle_node_t *node_left;
  merkle_node_t *node_right;
  uint8_t *hash;
  uint32_t hash_length;
};

int merkle_node_new(merkle_node_t **merkle_node_out, size_t range_start,
                    size_t range_end, merkle_node_t *node_left,
                    merkle_node_t *node_right) {
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
  *merkle_out = merkle;
  return 0;
}

int merkle_build_recursive(merkle_t *merkle, serializable_t **serializables,
                           size_t serializables_length,
                           merkle_node_t **merkle_node_out, size_t range_start,
                           size_t range_end) {
  if (range_start == range_end) {
    merkle_node_t *merkle_node;
    RETURN_IF_NEG(
        merkle_node_new(&merkle_node, range_start, range_end, NULL, NULL))
    serializable_t *serializable = serializables[range_start];
    buffer_t *buffer;
    RETURN_IF_NEG(buffer_new(&buffer))
    RETURN_IF_NEG(serializable->serialize(serializable, buffer))
    RETURN_IF_NEG(merkle->hash_buffer->hash(buffer, &merkle_node->hash,
                                            &merkle_node->hash_length))
    *merkle_node_out = merkle_node;
    RETURN_IF_NEG(buffer_free(buffer))
    return 0;
  }

  size_t mid = (range_start + range_end) >> 1;
  merkle_node_t *node_left = NULL, *node_right = NULL;
  RETURN_IF_NEG(merkle_build_recursive(merkle, serializables,
                                       serializables_length, &node_left,
                                       range_start, mid))
  RETURN_IF_NEG(merkle_build_recursive(merkle, serializables,
                                       serializables_length, &node_right,
                                       mid + 1, range_end))
  merkle_node_t *node_parent = NULL;
  RETURN_IF_NEG(merkle_node_new(&node_parent, range_start, range_end, node_left,
                                node_right))
  buffer_t *buffer;
  RETURN_IF_NEG(buffer_new(&buffer))
  if (node_left) {
    RETURN_IF_NEG(
        buffer_memcpy(buffer, node_left->hash, node_left->hash_length))
  }
  if (node_right) {
    RETURN_IF_NEG(
        buffer_memcpy(buffer, node_right->hash, node_right->hash_length))
  }
  RETURN_IF_NEG(merkle->hash_buffer->hash(buffer, &node_parent->hash,
                                          &node_parent->hash_length))
  RETURN_IF_NEG(buffer_free(buffer))
  *merkle_node_out = node_parent;
  return 0;
}

int merkle_build(merkle_t *merkle, serializable_t **serializables,
                 size_t serializables_length) {
  RETURN_IF_NEG(merkle_build_recursive(merkle, serializables,
                                       serializables_length, &merkle->root, 0,
                                       serializables_length - 1))
  return 0;
}

int merkle_walk(merkle_t *merkle, merkle_node_t *merkle_node) {

  printf("range: [%ld, %ld]\n", merkle_node->range_start,
         merkle_node->range_end);
  printf("hash: ");
  for (size_t i = 0; i < merkle_node->hash_length; i++) {
    printf("%02x", merkle_node->hash[i]);
  }
  printf("\nnode left address: %p\n", merkle_node->node_left);
  printf("node right address: %p\n\n", merkle_node->node_right);

  if (merkle_node->node_left) {
    RETURN_IF_NEG(merkle_walk(merkle, merkle_node->node_left))
  }
  if (merkle_node->node_right) {
    RETURN_IF_NEG(merkle_walk(merkle, merkle_node->node_right))
  }
}

int merkle_debug(merkle_t *merkle) {
  RETURN_IF_NEG(merkle_walk(merkle, merkle->root))
}