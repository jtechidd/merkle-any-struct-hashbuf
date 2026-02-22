#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <stdbool.h>
#include <stdio.h>

#include "alloc.h"
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

void player_serialize(serializable_t *, buffer_t *);

player_t *player_new(const char *name, uint32_t points) {
  player_t *player = xmalloc(sizeof(player_t));
  player->name = name;
  player->points = points;
  player->serializable.serialize = player_serialize;
  return player;
}

void player_serialize(serializable_t *serializable, buffer_t *buffer) {
  player_t *player = (player_t *)serializable;
  size_t name_len = strlen(player->name);
  buffer_memcpy(buffer, &name_len, sizeof(name_len));
  buffer_memcpy(buffer, (void *)player->name, name_len);
  buffer_memcpy(buffer, &player->points, sizeof(player->points));
}

void player_free(player_t *player) { free(player); }

typedef struct hash_buffer_sha256_t {
  hash_buffer_t hash_buffer;
} hash_buffer_sha256_t;

void hash_buffer_sha256_hash(hash_data_t *, buffer_t *);

hash_buffer_sha256_t *hash_buffer_sha256_new() {
  hash_buffer_sha256_t *hash_buffer_sha256 =
      xmalloc(sizeof(hash_buffer_sha256_t));
  hash_buffer_sha256->hash_buffer.hash = hash_buffer_sha256_hash;
  return hash_buffer_sha256;
}

void hash_buffer_sha256_hash(hash_data_t *hash_data_out, buffer_t *buffer) {
  int ret;
  EVP_MD_CTX *evp_md_ctx = NULL;
  if (!(evp_md_ctx = EVP_MD_CTX_new())) {
    return;
  }
  if (EVP_DigestInit_ex(evp_md_ctx, EVP_sha256(), NULL) != 1) {
    return;
  }
  uint8_t *buffer_byte_array = buffer_get_byte_array(buffer);
  size_t buffer_length = buffer_get_length(buffer);
  if (EVP_DigestUpdate(evp_md_ctx, buffer_byte_array, buffer_length) != 1) {
    return;
  }
  uint8_t *hash = NULL;
  uint32_t hash_length;
  if (!(hash = OPENSSL_malloc(EVP_MD_size(EVP_sha256())))) {
    return;
  }
  if (EVP_DigestFinal_ex(evp_md_ctx, hash, &hash_length) != 1) {
    return;
  }
  hash_data_set(hash_data_out, hash, hash_length);
  EVP_MD_CTX_free(evp_md_ctx);
}

void add_player(darray_t *players, const char *name, uint32_t points) {
  player_t *player = player_new(name, points);
  darray_add(players, player);
}

void proof_array_debug(darray_t *proof_array) {
  size_t length = darray_get_length(proof_array);

  for (size_t i = 0; i < length; i++) {
    hash_data_t *hash_data = darray_get_index(proof_array, i);
    hash_data_debug(hash_data);
  }
}

int main() {
  darray_t *players = darray_new();

  add_player(players, "Adam", 1234);
  add_player(players, "Eva", 5678);
  add_player(players, "JohnDoe", 100);
  add_player(players, "Foobar", 1000);
  add_player(players, "HelloWorld", 500);
  add_player(players, "Bug", 700);
  add_player(players, "Debug", 900);
  add_player(players, "Creator", 5000);

  hash_buffer_sha256_t *hash_buffer_256 = hash_buffer_sha256_new();

  merkle_t *merkle = merkle_new((hash_buffer_t *)hash_buffer_256);
  merkle_build(merkle, players);

  merkle_debug(merkle);

  darray_t *proof_array;

  size_t total_leaves = merkle_get_total_leaves(merkle);

  for (size_t i = 0; i < total_leaves; i++) {
    printf("Getting proof for leaf index: %ld\n", i);
    darray_t *proof_array = merkle_get_proof(merkle, i);
    proof_array_debug(proof_array);

    bool verify_ok = merkle_verify(merkle, proof_array);

    printf("Verifying proof result: %d\n\n", verify_ok);
  }
}
