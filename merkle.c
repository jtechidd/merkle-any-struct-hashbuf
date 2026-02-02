#include "merkle.h"
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

int merkle_node_new(merkle_node_t **merkle_node_out, size_t range_start, size_t range_end, merkle_node_t *node_left, merkle_node_t *node_right, hash_data_t *hash_data) {
  merkle_node_t *merkle_node = malloc(sizeof(merkle_node_t));
  if (!merkle_node) {
    return -1;
  }
  merkle_node->range_start = range_start;
  merkle_node->range_end = range_end;
  merkle_node->node_left = node_left;
  merkle_node->node_right = node_right;
  merkle_node->hash_data = hash_data;
  *merkle_node_out = merkle_node;
  return 0;
}

int merkle_node_free(merkle_node_t *merkle_node) {
  free(merkle_node->hash_data);
  free(merkle_node);
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
  int ret;
  int cmp_result;
  if ((ret = hash_data_cmp(&cmp_result, *hash_data_left, *hash_data_right)) < 0) {
    return ret;
  }
  if (cmp_result > 0) {
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
  int ret;

  if (range_start == range_end) {
    serializable_t *serializable;
    if ((ret = darray_get_index((void **)&serializable, darray, range_start)) < 0) {
      goto leave_ret;
    }

    buffer_t *buffer;
    if ((ret = buffer_new(&buffer)) < 0) {
      goto leave_ret;
    }
    if ((ret = serializable->serialize(serializable, buffer)) < 0) {
      goto leave_free_buffer;
    }

    hash_data_t *hash_data;
    if ((ret = hash_data_new(&hash_data)) < 0) {
      goto leave_free_buffer;
    }
    if ((merkle->hash_buffer->hash(hash_data, buffer)) < 0) {
      goto leave_free_hash_data;
    }

    merkle_node_t *merkle_node;
    if ((ret = merkle_node_new(&merkle_node, range_start, range_end, NULL, NULL, hash_data)) < 0) {
      goto leave_free_hash_data;
    }

    *merkle_node_out = merkle_node;
    merkle->total_nodes++;
    merkle->total_leaves++;
    return 0;

  leave_free_hash_data:
    hash_data_free(hash_data);
  leave_free_buffer:
    buffer_free(buffer);
  leave_ret:
    return ret;
  }

  size_t mid = (range_start + range_end) >> 1;

  merkle_node_t *node_left = NULL;
  if ((ret = merkle_build_recursive(&node_left, merkle, darray, range_start, mid)) < 0) {
    goto ret;
  }
  merkle_node_t *node_right = NULL;
  if ((ret = merkle_build_recursive(&node_right, merkle, darray, mid + 1, range_end)) < 0) {
    goto free_left_node;
  }

  buffer_t *buffer;
  if ((ret = buffer_new(&buffer)) < 0) {
    goto free_right_node;
  }

  hash_data_t *node_left_hash_data = node_left->hash_data;
  hash_data_t *node_right_hash_data = node_right->hash_data;
  if ((ret = sort_hash_data(&node_left_hash_data, &node_right_hash_data)) < 0) {
    goto free_buffer;
  }

  if ((ret = buffer_memcpy_from_hash_data(buffer, node_left_hash_data)) < 0) {
    goto free_buffer;
  }
  if ((ret = buffer_memcpy_from_hash_data(buffer, node_right_hash_data)) < 0) {
    goto free_buffer;
  }

  hash_data_t *hash_data;
  if ((ret = hash_data_new(&hash_data)) < 0) {
    goto free_buffer;
  }

  if ((ret = merkle->hash_buffer->hash(hash_data, buffer)) < 0) {
    goto free_hash_data;
  }

  merkle_node_t *node_parent = NULL;
  if ((ret = merkle_node_new(&node_parent, range_start, range_end, node_left, node_right, hash_data)) < 0) {
    goto free_hash_data;
  }

  *merkle_node_out = node_parent;
  merkle->total_nodes++;
  return 0;

free_hash_data:
  hash_data_free(hash_data);
free_buffer:
  buffer_free(buffer);
free_right_node:
  merkle_node_free(node_right);
free_left_node:
  merkle_node_free(node_left);
ret:
  return ret;
}

int merkle_build(merkle_t *merkle, darray_t *darray) {
  int ret;
  size_t darray_length;
  if ((ret = darray_get_length(&darray_length, darray))) {
    return ret;
  }
  if ((ret = merkle_build_recursive(&merkle->root, merkle, darray, 0, darray_length - 1) < 0)) {
    return ret;
  }
  return 0;
}

int merkle_get_proof_recursive(darray_t *proof_array_out, merkle_t *merkle, merkle_node_t *node, size_t index, size_t range_start, size_t range_end) {
  int ret;
  if (range_start == range_end) {
    if ((ret = darray_add(proof_array_out, node->hash_data)) < 0) {
      return ret;
    }
    return 0;
  }
  size_t mid = (range_start + range_end) >> 1;
  if (index <= mid) {
    if ((ret = merkle_get_proof_recursive(proof_array_out, merkle, node->node_left, index, range_start, mid))) {
      return ret;
    }
    if ((ret = darray_add(proof_array_out, node->node_right->hash_data))) {
      return ret;
    }
  } else {
    if ((ret = merkle_get_proof_recursive(proof_array_out, merkle, node->node_right, index, mid + 1, range_end))) {
      return ret;
    }
    if ((ret = darray_add(proof_array_out, node->node_left->hash_data))) {
      return ret;
    }
  }
  return 0;
}

int merkle_get_proof(darray_t *proof_array_out, merkle_t *merkle, size_t index) {
  int ret;
  if (!(index >= 0 && index < merkle->total_leaves)) {
    return -1;
  }
  if ((ret = merkle_get_proof_recursive(proof_array_out, merkle, merkle->root, index, 0, merkle->total_leaves - 1) < 0)) {
    return ret;
  }
  return 0;
}

int merkle_verify(bool *verify_ok_out, merkle_t *merkle, darray_t *proof_array) {
  int ret;
  size_t proof_array_length;
  hash_data_t *accumulated_hash_data;
  hash_data_t *first_proof_hash_data;
  hash_data_t *hash_data;
  hash_data_t *left_hash_data = accumulated_hash_data;
  hash_data_t *right_hash_data = hash_data;
  buffer_t *buffer;
  hash_data_t *new_accumulated_hash_data;

  if ((ret = darray_get_length(&proof_array_length, proof_array)) < 0) {
    goto ret;
  }

  if ((ret = darray_get_index((void **)&first_proof_hash_data, proof_array, 0))) {
    goto ret;
  }
  if ((ret = hash_data_clone(&accumulated_hash_data, first_proof_hash_data))) {
    goto ret;
  }

  for (size_t i = 1; i < proof_array_length; i++) {
    if ((ret = darray_get_index((void **)&hash_data, proof_array, i)) < 0) {
      goto free_accumulated_hash_data;
    }

    if ((ret = sort_hash_data(&left_hash_data, &right_hash_data))) {
      goto free_accumulated_hash_data;
    }

    if ((ret = buffer_new(&buffer)) < 0) {
      goto free_accumulated_hash_data;
    }

    if ((ret = buffer_memcpy_from_hash_data(buffer, left_hash_data)) < 0) {
      goto free_buffer;
    }
    if ((ret = buffer_memcpy_from_hash_data(buffer, right_hash_data)) < 0) {
      goto free_buffer;
    }

    if ((ret = hash_data_new(&new_accumulated_hash_data)) < 0) {
      goto free_buffer;
    }
    if ((ret = merkle->hash_buffer->hash(new_accumulated_hash_data, buffer)) < 0) {
      goto free_new_accumulated_hash_data;
    }

    buffer_free(buffer);
    hash_data_free(accumulated_hash_data);

    accumulated_hash_data = new_accumulated_hash_data;
  }

  int cmp_result;
  if ((ret = hash_data_cmp(&cmp_result, accumulated_hash_data, merkle->root->hash_data)) < 0) {
    goto free_accumulated_hash_data;
  }

  *verify_ok_out = !cmp_result;
  hash_data_free(accumulated_hash_data);
  return 0;

free_new_accumulated_hash_data:
  hash_data_free(new_accumulated_hash_data);
free_buffer:
  buffer_free(buffer);
free_accumulated_hash_data:
  hash_data_free(accumulated_hash_data);
ret:
  return ret;
}

int merkle_walk(merkle_t *merkle, merkle_node_t *merkle_node) {
  int ret;

  printf("Range: [%ld, %ld]\n", merkle_node->range_start, merkle_node->range_end);
  uint8_t *hash;
  uint32_t hash_length;
  if ((ret = hash_data_get_hash(&hash, merkle_node->hash_data)) < 0) {
    return ret;
  }
  if ((ret = hash_data_get_hash_length(&hash_length, merkle_node->hash_data)) < 0) {
    return ret;
  }
  for (size_t i = 0; i < hash_length; i++) {
    printf("%02x", hash[i]);
  }
  printf("\nNode left address: %p\n", merkle_node->node_left);
  printf("Node right address: %p\n\n", merkle_node->node_right);

  if (merkle_node->node_left) {
    if ((ret = merkle_walk(merkle, merkle_node->node_left)) < 0) {
      return ret;
    }
  }
  if (merkle_node->node_right) {
    if ((ret = merkle_walk(merkle, merkle_node->node_right)) < 0) {
      return ret;
    }
  }
  return 0;
}

int merkle_debug(merkle_t *merkle) {
  int ret;

  printf("Total nodes: %ld\n", merkle->total_nodes);
  printf("Total leaves: %ld\n\n", merkle->total_leaves);

  if ((ret = merkle_walk(merkle, merkle->root)) < 0) {
    return ret;
  }

  return 0;
}