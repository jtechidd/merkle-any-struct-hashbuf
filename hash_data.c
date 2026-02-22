#include "hash_data.h"
#include "alloc.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct hash_data_t {
  uint8_t *hash;
  uint32_t hash_length;
} hash_data_t;

hash_data_t *hash_data_new() {
  hash_data_t *hash_data = xmalloc(sizeof(hash_data_t));
  return hash_data;
}

hash_data_t *hash_data_clone(hash_data_t *hash_data) {
  hash_data_t *cloned_hash_data = xmalloc(sizeof(hash_data_t));
  cloned_hash_data->hash = xmalloc(hash_data->hash_length * sizeof(uint8_t));
  memcpy(cloned_hash_data->hash, hash_data->hash, hash_data->hash_length);
  cloned_hash_data->hash_length = hash_data->hash_length;
  return cloned_hash_data;
}

uint8_t *hash_data_get_hash(hash_data_t *hash_data) { return hash_data->hash; }

uint32_t hash_data_get_hash_length(hash_data_t *hash_data) {
  return hash_data->hash_length;
}

void hash_data_set(hash_data_t *hash_data, uint8_t *hash,
                   uint32_t hash_length) {
  hash_data->hash = hash;
  hash_data->hash_length = hash_length;
}

int hash_data_cmp(hash_data_t *left_hash_data, hash_data_t *right_hash_data) {
  assert(left_hash_data->hash_length == right_hash_data->hash_length);
  return strncmp((const char *)left_hash_data->hash,
                 (const char *)right_hash_data->hash,
                 left_hash_data->hash_length);
}

void hash_data_debug(hash_data_t *hash_data) {
  for (size_t i = 0; i < hash_data->hash_length; i++) {
    printf("%02x", hash_data->hash[i]);
  }
  printf("\n");
}

void hash_data_free(hash_data_t *hash_data) {
  if (hash_data->hash != NULL) {
    // TODO: Try a way to inject custom free function for hash
    free(hash_data->hash);
    hash_data->hash = NULL;
  }
  free(hash_data);
}