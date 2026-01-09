#define _POSIX_C_SOURCE 200809L
#include "cache.h"
#include <pthread.h>

// Cache entry
typedef struct cache_entry {
    char* key;
    void* value;
    size_t value_size;
    uint64_t timestamp;
    uint64_t ttl_ms;
    uint64_t access_count;
    struct cache_entry* prev;
    struct cache_entry* next;
    struct cache_entry* hash_next;
} cache_entry_t;

struct cache {
    cache_entry_t** hash_table;
    size_t hash_size;
    cache_entry_t* head;  // Most recently used
    cache_entry_t* tail;  // Least recently used
    size_t size;
    size_t max_size;
    eviction_policy_t policy;
    pthread_rwlock_t lock;
    
    // Statistics
    size_t hits;
    size_t misses;
    size_t evictions;
};

static uint32_t hash_key(const char* key) {
    uint32_t hash = 5381;
    int c;
    while ((c = *key++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

static void remove_from_list(cache_t* cache, cache_entry_t* entry) {
    if (entry->prev) {
        entry->prev->next = entry->next;
    } else {
        cache->head = entry->next;
    }
    
    if (entry->next) {
        entry->next->prev = entry->prev;
    } else {
        cache->tail = entry->prev;
    }
}

static void add_to_head(cache_t* cache, cache_entry_t* entry) {
    entry->prev = NULL;
    entry->next = cache->head;
    
    if (cache->head) {
        cache->head->prev = entry;
    }
    cache->head = entry;
    
    if (!cache->tail) {
        cache->tail = entry;
    }
}

static void move_to_head(cache_t* cache, cache_entry_t* entry) {
    if (cache->head == entry) return;
    
    remove_from_list(cache, entry);
    add_to_head(cache, entry);
}

static cache_entry_t* find_entry(cache_t* cache, const char* key) {
    uint32_t hash = hash_key(key);
    size_t bucket = hash % cache->hash_size;
    
    cache_entry_t* entry = cache->hash_table[bucket];
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            // Check TTL
            if (entry->ttl_ms > 0) {
                uint64_t now = get_timestamp_ms();
                if (now - entry->timestamp > entry->ttl_ms) {
                    return NULL;  // Expired
                }
            }
            return entry;
        }
        entry = entry->hash_next;
    }
    
    return NULL;
}

static void remove_entry(cache_t* cache, cache_entry_t* entry) {
    // Remove from hash table
    uint32_t hash = hash_key(entry->key);
    size_t bucket = hash % cache->hash_size;
    
    cache_entry_t* current = cache->hash_table[bucket];
    cache_entry_t* prev = NULL;
    
    while (current) {
        if (current == entry) {
            if (prev) {
                prev->hash_next = current->hash_next;
            } else {
                cache->hash_table[bucket] = current->hash_next;
            }
            break;
        }
        prev = current;
        current = current->hash_next;
    }
    
    // Remove from list
    remove_from_list(cache, entry);
    
    // Free memory
    safe_free((void**)&entry->key);
    safe_free((void**)&entry->value);
    safe_free((void**)&entry);
    
    cache->size--;
}

static cache_entry_t* find_victim(cache_t* cache) {
    if (cache->policy == EVICTION_LRU) {
        return cache->tail;  // Least recently used
    } else {  // EVICTION_LFU
        // Find least frequently used
        cache_entry_t* victim = cache->head;
        cache_entry_t* current = cache->head;
        
        while (current) {
            if (current->access_count < victim->access_count) {
                victim = current;
            }
            current = current->next;
        }
        
        return victim;
    }
}

static void evict_if_needed(cache_t* cache) {
    while (cache->size >= cache->max_size) {
        cache_entry_t* victim = find_victim(cache);
        if (victim) {
            remove_entry(cache, victim);
            cache->evictions++;
        } else {
            break;
        }
    }
}

cache_t* cache_create(size_t max_size, eviction_policy_t policy) {
    cache_t* cache = safe_calloc(1, sizeof(cache_t));
    cache->max_size = max_size;
    cache->policy = policy;
    cache->hash_size = max_size * 2;  // 2x for better distribution
    cache->hash_table = safe_calloc(cache->hash_size, sizeof(cache_entry_t*));
    pthread_rwlock_init(&cache->lock, NULL);
    return cache;
}

void cache_destroy(cache_t* cache) {
    if (!cache) return;
    
    cache_clear(cache);
    
    safe_free((void**)&cache->hash_table);
    pthread_rwlock_destroy(&cache->lock);
    safe_free((void**)&cache);
}

int cache_put(cache_t* cache, const char* key, const void* value, size_t value_size) {
    return cache_put_with_ttl(cache, key, value, value_size, 0);
}

int cache_put_with_ttl(cache_t* cache, const char* key, const void* value, 
                       size_t value_size, uint64_t ttl_ms) {
    if (!cache || !key || !value || value_size == 0) {
        return ERROR_INVALID_PARAM;
    }
    
    pthread_rwlock_wrlock(&cache->lock);
    
    // Check if key exists
    cache_entry_t* existing = find_entry(cache, key);
    if (existing) {
        // Update existing entry
        safe_free((void**)&existing->value);
        existing->value = safe_malloc(value_size);
        memcpy(existing->value, value, value_size);
        existing->value_size = value_size;
        existing->timestamp = get_timestamp_ms();
        existing->ttl_ms = ttl_ms;
        
        if (cache->policy == EVICTION_LRU) {
            move_to_head(cache, existing);
        }
        
        pthread_rwlock_unlock(&cache->lock);
        return SUCCESS;
    }
    
    // Evict if needed
    evict_if_needed(cache);
    
    // Create new entry
    cache_entry_t* entry = safe_calloc(1, sizeof(cache_entry_t));
    entry->key = safe_strdup(key);
    entry->value = safe_malloc(value_size);
    memcpy(entry->value, value, value_size);
    entry->value_size = value_size;
    entry->timestamp = get_timestamp_ms();
    entry->ttl_ms = ttl_ms;
    entry->access_count = 0;
    
    // Add to hash table
    uint32_t hash = hash_key(key);
    size_t bucket = hash % cache->hash_size;
    entry->hash_next = cache->hash_table[bucket];
    cache->hash_table[bucket] = entry;
    
    // Add to list
    add_to_head(cache, entry);
    
    cache->size++;
    
    pthread_rwlock_unlock(&cache->lock);
    return SUCCESS;
}

int cache_get(cache_t* cache, const char* key, void** value, size_t* value_size) {
    if (!cache || !key) {
        return ERROR_INVALID_PARAM;
    }
    
    pthread_rwlock_wrlock(&cache->lock);
    
    cache_entry_t* entry = find_entry(cache, key);
    if (!entry) {
        cache->misses++;
        pthread_rwlock_unlock(&cache->lock);
        return ERROR_NOT_FOUND;
    }
    
    cache->hits++;
    entry->access_count++;
    
    if (cache->policy == EVICTION_LRU) {
        move_to_head(cache, entry);
    }
    
    if (value) {
        *value = safe_malloc(entry->value_size);
        memcpy(*value, entry->value, entry->value_size);
    }
    
    if (value_size) {
        *value_size = entry->value_size;
    }
    
    pthread_rwlock_unlock(&cache->lock);
    return SUCCESS;
}

int cache_delete(cache_t* cache, const char* key) {
    if (!cache || !key) {
        return ERROR_INVALID_PARAM;
    }
    
    pthread_rwlock_wrlock(&cache->lock);
    
    cache_entry_t* entry = find_entry(cache, key);
    if (!entry) {
        pthread_rwlock_unlock(&cache->lock);
        return ERROR_NOT_FOUND;
    }
    
    remove_entry(cache, entry);
    
    pthread_rwlock_unlock(&cache->lock);
    return SUCCESS;
}

int cache_exists(cache_t* cache, const char* key) {
    if (!cache || !key) {
        return 0;
    }
    
    pthread_rwlock_rdlock(&cache->lock);
    cache_entry_t* entry = find_entry(cache, key);
    pthread_rwlock_unlock(&cache->lock);
    
    return entry != NULL;
}

void cache_clear(cache_t* cache) {
    if (!cache) return;
    
    pthread_rwlock_wrlock(&cache->lock);
    
    cache_entry_t* current = cache->head;
    while (current) {
        cache_entry_t* next = current->next;
        safe_free((void**)&current->key);
        safe_free((void**)&current->value);
        safe_free((void**)&current);
        current = next;
    }
    
    memset(cache->hash_table, 0, cache->hash_size * sizeof(cache_entry_t*));
    cache->head = NULL;
    cache->tail = NULL;
    cache->size = 0;
    
    pthread_rwlock_unlock(&cache->lock);
}

int cache_get_stats(cache_t* cache, cache_stats_t* stats) {
    if (!cache || !stats) {
        return ERROR_INVALID_PARAM;
    }
    
    pthread_rwlock_rdlock(&cache->lock);
    
    stats->size = cache->size;
    stats->max_size = cache->max_size;
    stats->hits = cache->hits;
    stats->misses = cache->misses;
    stats->evictions = cache->evictions;
    
    pthread_rwlock_unlock(&cache->lock);
    
    return SUCCESS;
}
