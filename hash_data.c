#include "hash_data.h"

#include <stdio.h>
#include <string.h>

typedef struct hash_data_t {
  uint8_t *hash;
  uint32_t hash_length;
} hash_data_t;

int hash_data_new(hash_data_t **hash_data_out) {
  hash_data_t *hash_data = malloc(sizeof(hash_data_t));
  if (!hash_data) {
    return -1;
  }
  *hash_data_out = hash_data;
  return 0;
}

int hash_data_clone(hash_data_t **hash_data_out, hash_data_t *hash_data) {
  hash_data_t *cloned_hash_data = malloc(sizeof(hash_data_t));
  if (!cloned_hash_data) {
    return -1;
  }
  cloned_hash_data->hash = malloc(hash_data->hash_length * sizeof(uint8_t));
  if (!cloned_hash_data->hash) {
    free(cloned_hash_data);
    return -1;
  }
  memcpy(cloned_hash_data->hash, hash_data->hash, hash_data->hash_length);
  cloned_hash_data->hash_length = hash_data->hash_length;
  *hash_data_out = cloned_hash_data;
  return 0;
}

int hash_data_get_hash(uint8_t **hash_out, hash_data_t *hash_data) {
  *hash_out = hash_data->hash;
  return 0;
}

int hash_data_get_hash_length(uint32_t *hash_length_out, hash_data_t *hash_data) {
  *hash_length_out = hash_data->hash_length;
  return 0;
}

int hash_data_set(hash_data_t *hash_data, uint8_t *hash, uint32_t hash_length) {
  hash_data->hash = hash;
  hash_data->hash_length = hash_length;
  return 0;
}

int hash_data_cmp(int *cmp_result_out, hash_data_t *left_hash_data, hash_data_t *right_hash_data) {
  if (left_hash_data->hash_length != right_hash_data->hash_length) {
    return -1;
  }
  *cmp_result_out = strncmp((const char *)left_hash_data->hash, (const char *)right_hash_data->hash, left_hash_data->hash_length);
  return 0;
}

int hash_data_debug(hash_data_t *hash_data) {
  for (size_t i = 0; i < hash_data->hash_length; i++) {
    printf("%02x", hash_data->hash[i]);
  }
  printf("\n");
  return 0;
}

int hash_data_free(hash_data_t *hash_data) {
  if (hash_data->hash != NULL) {
    // TODO: Try a way to inject custom free function for hash
    free(hash_data->hash);
    hash_data->hash = NULL;
  }
  free(hash_data);
  return 0;
}