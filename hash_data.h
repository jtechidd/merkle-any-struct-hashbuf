#ifndef hash_data_h
#define hash_data_h

#include <stdint.h>
#include <stdlib.h>

typedef struct hash_data_t hash_data_t;

hash_data_t* hash_data_new();
hash_data_t* hash_data_clone(hash_data_t *);
uint8_t* hash_data_get_hash(hash_data_t *);
uint32_t hash_data_get_hash_length(hash_data_t *);
void hash_data_set(hash_data_t *, uint8_t *, uint32_t);
int hash_data_cmp(hash_data_t *, hash_data_t *);
void hash_data_debug(hash_data_t *);
void hash_data_free(hash_data_t *);

#endif