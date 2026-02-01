#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <stdbool.h>
#include <stdio.h>

#include "buffer.h"
#include "common.h"
#include "darray.h"
#include "hash_buffer.h"
#include "hash_data.h"
#include "merkle.h"
#include "serializable.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct player_t {
  serializable_t serializable;

  const char *name;
  uint32_t points;
} player_t;

int player_serialize(serializable_t *, buffer_t *);

int player_new(player_t **player_out, const char *name, uint32_t points) {
  player_t *player = malloc(sizeof(player_t));
  if (!player) {
    *player_out = NULL;
    return -1;
  }
  player->name = name;
  player->points = points;
  player->serializable.serialize = player_serialize;
  *player_out = player;
  return 0;
}

int player_serialize(serializable_t *serializable, buffer_t *buffer) {
  player_t *player = (player_t *)serializable;
  size_t name_len = strlen(player->name);
  RETURN_IF_ERROR(buffer_memcpy(buffer, &name_len, sizeof(name_len)))
  RETURN_IF_ERROR(buffer_memcpy(buffer, (void *)player->name, name_len))
  RETURN_IF_ERROR(buffer_memcpy(buffer, &player->points, sizeof(player->points)))
  return 0;
}

typedef struct hash_buffer_sha256_t {
  hash_buffer_t hash_buffer;
} hash_buffer_sha256_t;

int hash_buffer_sha256_hash(hash_data_t *, buffer_t *);

int hash_buffer_sha256_new(hash_buffer_sha256_t **hash_buffer_sha256_out) {
  hash_buffer_sha256_t *hash_buffer_sha256 = malloc(sizeof(hash_buffer_sha256_t));
  hash_buffer_sha256->hash_buffer.hash = hash_buffer_sha256_hash;
  *hash_buffer_sha256_out = hash_buffer_sha256;
  return 0;
}

int hash_buffer_sha256_hash(hash_data_t *hash_data_out, buffer_t *buffer) {
  EVP_MD_CTX *evp_md_ctx;
  if (!(evp_md_ctx = EVP_MD_CTX_new())) {
    return -1;
  }
  if (EVP_DigestInit_ex(evp_md_ctx, EVP_sha256(), NULL) != 1) {
    return -1;
  }
  uint8_t *buffer_byte_array;
  size_t buffer_length;
  RETURN_IF_ERROR(buffer_get_byte_array(&buffer_byte_array, buffer))
  RETURN_IF_ERROR(buffer_get_length(&buffer_length, buffer))
  if (EVP_DigestUpdate(evp_md_ctx, buffer_byte_array, buffer_length) != 1) {
    return -1;
  }
  uint8_t *hash;
  uint32_t hash_length;
  if (!(hash = OPENSSL_malloc(EVP_MD_size(EVP_sha256())))) {
    return -1;
  }
  if (EVP_DigestFinal_ex(evp_md_ctx, hash, &hash_length) != 1) {
    return -1;
  }
  hash_data_set(hash_data_out, hash, hash_length);
  EVP_MD_CTX_free(evp_md_ctx);
  return 0;
}

int add_player(darray_t *players, const char *name, uint32_t points) {
  player_t *player;
  RETURN_IF_ERROR(player_new(&player, name, points))
  RETURN_IF_ERROR(darray_add(players, player))
  return 0;
}

int proof_array_debug(darray_t *proof_array) {
  size_t length;
  RETURN_IF_ERROR(darray_get_length(&length, proof_array))
  for (size_t i = 0; i < length; i++) {
    hash_data_t *hash_data;
    RETURN_IF_ERROR(darray_get_index((void **)&hash_data, proof_array, i))
    RETURN_IF_ERROR(hash_data_debug(hash_data))
  }
  return 0;
}

int main() {
  darray_t *players;
  RETURN_IF_ERROR(darray_new(&players))

  RETURN_IF_ERROR(add_player(players, "Adam", 1234))
  RETURN_IF_ERROR(add_player(players, "Eva", 5678))
  RETURN_IF_ERROR(add_player(players, "JohnDoe", 100))
  RETURN_IF_ERROR(add_player(players, "Foobar", 1000))
  RETURN_IF_ERROR(add_player(players, "HelloWorld", 500))
  RETURN_IF_ERROR(add_player(players, "Bug", 700))
  RETURN_IF_ERROR(add_player(players, "Debug", 900))
  RETURN_IF_ERROR(add_player(players, "Creator", 5000))

  hash_buffer_sha256_t *hash_buffer_256;
  RETURN_IF_ERROR(hash_buffer_sha256_new(&hash_buffer_256))

  merkle_t *merkle;
  RETURN_IF_ERROR(merkle_new(&merkle, (hash_buffer_t *)hash_buffer_256))
  RETURN_IF_ERROR(merkle_build(merkle, players))

  RETURN_IF_ERROR(merkle_debug(merkle))

  darray_t *proof_array;

  size_t total_leaves;
  merkle_get_total_leaves(&total_leaves, merkle);

  for (size_t i = 0; i < total_leaves; i++) {
    printf("Getting proof for leaf index: %ld\n", i);
    RETURN_IF_ERROR(darray_new(&proof_array))
    RETURN_IF_ERROR(merkle_get_proof(proof_array, merkle, i))
    RETURN_IF_ERROR(proof_array_debug(proof_array))

    bool verify_ok;
    RETURN_IF_ERROR(merkle_verify(&verify_ok, merkle, proof_array))

    printf("Verifying proof result: %d\n\n", verify_ok);
  }
}