#ifndef CACHE_H
#define CACHE_H

#include "common.h"

// Cache eviction policies
typedef enum {
    EVICTION_LRU,   // Least Recently Used
    EVICTION_LFU    // Least Frequently Used
} eviction_policy_t;

typedef struct cache cache_t;

// Cache functions
cache_t* cache_create(size_t max_size, eviction_policy_t policy);
void cache_destroy(cache_t* cache);

int cache_put(cache_t* cache, const char* key, const void* value, size_t value_size);
int cache_put_with_ttl(cache_t* cache, const char* key, const void* value, 
                       size_t value_size, uint64_t ttl_ms);
int cache_get(cache_t* cache, const char* key, void** value, size_t* value_size);
int cache_delete(cache_t* cache, const char* key);
int cache_exists(cache_t* cache, const char* key);
void cache_clear(cache_t* cache);

// Statistics
typedef struct {
    size_t size;
    size_t max_size;
    size_t hits;
    size_t misses;
    size_t evictions;
} cache_stats_t;

int cache_get_stats(cache_t* cache, cache_stats_t* stats);

#endif // CACHE_H
