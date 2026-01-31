#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <stdio.h>

#include "buffer.h"
#include "common.h"
#include "hash_buffer.h"
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

int player_new(player_t **player_out, const char *name, uint16_t points) {
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
  RETURN_IF_NEG(buffer_memcpy(buffer, &name_len, sizeof(name_len)))
  RETURN_IF_NEG(buffer_memcpy(buffer, (void *)player->name, name_len))
  RETURN_IF_NEG(buffer_memcpy(buffer, &player->points, sizeof(player->points)))
  return 0;
}

typedef struct hash_buffer_sha256_t {
  hash_buffer_t hash_buffer;
} hash_buffer_sha256_t;

int hash_buffer_sha256_hash(buffer_t *, uint8_t **, unsigned int *);

int hash_buffer_sha256_new(hash_buffer_sha256_t **hash_buffer_sha256_out) {
  hash_buffer_sha256_t *hash_buffer_sha256 =
      malloc(sizeof(hash_buffer_sha256_t));
  hash_buffer_sha256->hash_buffer.hash = hash_buffer_sha256_hash;
  *hash_buffer_sha256_out = hash_buffer_sha256;
  return 0;
}

int hash_buffer_sha256_hash(buffer_t *buffer, uint8_t **hash,
                            unsigned int *hash_length) {
  EVP_MD_CTX *evp_md_ctx;
  if (!(evp_md_ctx = EVP_MD_CTX_new())) {
    return -1;
  }
  if (EVP_DigestInit_ex(evp_md_ctx, EVP_sha256(), NULL) != 1) {
    return -1;
  }
  uint8_t *buffer_byte_array;
  size_t buffer_length;
  RETURN_IF_NEG(buffer_get_byte_array(buffer, &buffer_byte_array))
  RETURN_IF_NEG(buffer_get_length(buffer, &buffer_length))
  if (EVP_DigestUpdate(evp_md_ctx, buffer_byte_array, buffer_length) != 1) {
    return -1;
  }
  if (!(*hash = (uint8_t *)OPENSSL_malloc(EVP_MD_size(EVP_sha256())))) {
    return -1;
  }
  if (EVP_DigestFinal_ex(evp_md_ctx, *hash, (unsigned int *)hash_length) != 1) {
    return -1;
  }
  EVP_MD_CTX_free(evp_md_ctx);
  return 0;
}

int main() {
  size_t players_len = 8;
  serializable_t **players = malloc(players_len * sizeof(void *));
  RETURN_IF_NEG(player_new((player_t **)&players[0], "Adam", 1234))
  RETURN_IF_NEG(player_new((player_t **)&players[1], "Eva", 5678))
  RETURN_IF_NEG(player_new((player_t **)&players[2], "JohnDoe", 100))
  RETURN_IF_NEG(player_new((player_t **)&players[3], "Foobar", 1000))
  RETURN_IF_NEG(player_new((player_t **)&players[4], "HelloWorld", 500))
  RETURN_IF_NEG(player_new((player_t **)&players[5], "Bug", 700))
  RETURN_IF_NEG(player_new((player_t **)&players[6], "Debug", 900))
  RETURN_IF_NEG(player_new((player_t **)&players[7], "Creator", 5000))

  hash_buffer_sha256_t *hash_buffer_256;
  RETURN_IF_NEG(hash_buffer_sha256_new(&hash_buffer_256))

  merkle_t *merkle;
  RETURN_IF_NEG(merkle_new(&merkle, (hash_buffer_t *)hash_buffer_256));
  RETURN_IF_NEG(merkle_build(merkle, players, players_len));

  RETURN_IF_NEG(merkle_debug(merkle));
}