#ifndef FNV_HASH_
#define FNV_OFFSET 14695981039346656037U
#define FNV_PRIME 1099511628211U
#define FNV_HASH_

#include <stdint.h>

uint64_t fnv_hash(uint64_t starting_hash, void *data, char type);
uint64_t fnv_hash_byte(uint64_t hash, char *data);

#endif //FNV_HASH_
