#include "hash_data.h"

#include <stdio.h>
#include <string.h>

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

int hash_data_debug(hash_data_t *hash_data) {
  for (size_t i = 0; i < hash_data->hash_length; i++) {
    printf("%02x", hash_data->hash[i]);
  }
  printf("\n");
  return 0;
}

int hash_data_free(hash_data_t *hash_data) {
  // TODO: Try a way to inject custom free function for hash
  free(hash_data->hash);
  free(hash_data);
  return 0;
}