#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <stdbool.h>
#include <stdio.h>

#include "buffer.h"
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
  int ret;
  player_t *player = (player_t *)serializable;
  size_t name_len = strlen(player->name);
  if ((ret = buffer_memcpy(buffer, &name_len, sizeof(name_len)) < 0)) {
    return ret;
  }
  if ((ret = buffer_memcpy(buffer, (void *)player->name, name_len)) < 0) {
    return ret;
  }
  if ((ret = buffer_memcpy(buffer, &player->points, sizeof(player->points)) < 0)) {
    return ret;
  }
  return 0;
}

int player_free(player_t *player) {
  free(player);
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
  int ret;
  EVP_MD_CTX *evp_md_ctx;
  if (!(evp_md_ctx = EVP_MD_CTX_new())) {
    return -1;
  }
  if (EVP_DigestInit_ex(evp_md_ctx, EVP_sha256(), NULL) != 1) {
    ret = -1;
    goto free_evp_md_ctx;
  }
  uint8_t *buffer_byte_array;
  size_t buffer_length;
  if ((ret = buffer_get_byte_array(&buffer_byte_array, buffer)) < 0) {
    goto free_evp_md_ctx;
  }
  if ((ret = buffer_get_length(&buffer_length, buffer)) < 0) {
    goto free_evp_md_ctx;
  }
  if (EVP_DigestUpdate(evp_md_ctx, buffer_byte_array, buffer_length) != 1) {
    ret = -1;
    goto free_evp_md_ctx;
  }
  uint8_t *hash;
  uint32_t hash_length;
  if (!(hash = OPENSSL_malloc(EVP_MD_size(EVP_sha256())))) {
    ret = -1;
    goto free_evp_md_ctx;
  }
  if (EVP_DigestFinal_ex(evp_md_ctx, hash, &hash_length) != 1) {
    ret = -1;
    goto free_hash;
  }
  hash_data_set(hash_data_out, hash, hash_length);
  EVP_MD_CTX_free(evp_md_ctx);
  return 0;
free_hash:
  OPENSSL_free(hash);
free_evp_md_ctx:
  EVP_MD_CTX_free(evp_md_ctx);
ret:
  return ret;
}

int add_player(darray_t *players, const char *name, uint32_t points) {
  int ret;
  player_t *player;
  if ((ret = player_new(&player, name, points)) < 0) {
    return ret;
  }
  if ((ret = darray_add(players, player)) < 0) {
    player_free(player);
    return ret;
  }
  return 0;
}

int proof_array_debug(darray_t *proof_array) {
  int ret;
  size_t length;
  if ((ret = darray_get_length(&length, proof_array)) < 0) {
    return ret;
  }
  for (size_t i = 0; i < length; i++) {
    hash_data_t *hash_data;
    if ((ret = darray_get_index((void **)&hash_data, proof_array, i)) < 0) {
      return ret;
    }
    if ((ret = hash_data_debug(hash_data)) < 0) {
      return ret;
    }
  }
  return 0;
}

int main() {
  darray_t *players;
  darray_new(&players);

  add_player(players, "Adam", 1234);
  add_player(players, "Eva", 5678);
  add_player(players, "JohnDoe", 100);
  add_player(players, "Foobar", 1000);
  add_player(players, "HelloWorld", 500);
  add_player(players, "Bug", 700);
  add_player(players, "Debug", 900);
  add_player(players, "Creator", 5000);

  hash_buffer_sha256_t *hash_buffer_256;
  hash_buffer_sha256_new(&hash_buffer_256);

  merkle_t *merkle;
  merkle_new(&merkle, (hash_buffer_t *)hash_buffer_256);
  merkle_build(merkle, players);

  merkle_debug(merkle);

  darray_t *proof_array;

  size_t total_leaves;
  merkle_get_total_leaves(&total_leaves, merkle);

  for (size_t i = 0; i < total_leaves; i++) {
    printf("Getting proof for leaf index: %ld\n", i);
    darray_new(&proof_array);
    merkle_get_proof(proof_array, merkle, i);
    proof_array_debug(proof_array);

    bool verify_ok;
    merkle_verify(&verify_ok, merkle, proof_array);

    printf("Verifying proof result: %d\n\n", verify_ok);
  }
}
