#ifndef CACHE_STRATEGIES_H
#define CACHE_STRATEGIES_H

#include "common.h"
#include <stdint.h>
#include <stdbool.h>

// =============================================================================
// CACHING STRATEGIES & CACHE CORRECTNESS
// =============================================================================
// This module provides templates for:
// - Redis/memory cache implementations
// - TTL (Time To Live) management
// - Cache invalidation strategies
// - Cache stampede prevention
// - Read-through vs write-through caching
// - Cache warming and preloading
// - Distributed cache coordination

// Forward declarations
typedef struct cache_instance cache_instance_t;
typedef struct cache_entry cache_entry_t;
typedef struct cache_cluster cache_cluster_t;

// =============================================================================
// CACHE TYPES & STRATEGIES
// =============================================================================

typedef enum {
    CACHE_TYPE_MEMORY,       // In-memory cache (local)
    CACHE_TYPE_REDIS,        // Redis cache
    CACHE_TYPE_MEMCACHED,    // Memcached
    CACHE_TYPE_DISTRIBUTED   // Distributed cache (e.g., Hazelcast)
} cache_type_t;

typedef enum {
    CACHE_STRATEGY_LAZY,         // Load on demand (cache-aside)
    CACHE_STRATEGY_READ_THROUGH, // Load through cache automatically
    CACHE_STRATEGY_WRITE_THROUGH,// Write to cache and DB synchronously
    CACHE_STRATEGY_WRITE_BEHIND, // Write to cache, async to DB
    CACHE_STRATEGY_WRITE_AROUND  // Write directly to DB, invalidate cache
} cache_strategy_t;

typedef enum {
    CACHE_EVICTION_LRU,      // Least Recently Used
    CACHE_EVICTION_LFU,      // Least Frequently Used
    CACHE_EVICTION_FIFO,     // First In First Out
    CACHE_EVICTION_RANDOM,   // Random eviction
    CACHE_EVICTION_TTL       // Time-based eviction only
} cache_eviction_policy_t;

// =============================================================================
// CACHE CONFIGURATION
// =============================================================================

typedef struct {
    cache_type_t type;
    cache_strategy_t strategy;
    cache_eviction_policy_t eviction_policy;
    size_t max_size;         // Maximum number of entries
    size_t max_memory_mb;    // Maximum memory in MB
    uint64_t default_ttl_ms; // Default TTL in milliseconds
    bool enable_compression;
    bool enable_encryption;
    char redis_host[256];
    int redis_port;
    char redis_password[256];
    int redis_db;
} cache_config_t;

// Cache instance management
cache_instance_t* cache_instance_create(cache_config_t* config);
void cache_instance_destroy(cache_instance_t* cache);
int cache_instance_connect(cache_instance_t* cache);
int cache_instance_disconnect(cache_instance_t* cache);

// =============================================================================
// BASIC CACHE OPERATIONS
// =============================================================================

// Set/Get operations
int cache_set(cache_instance_t* cache, const char* key, const void* value, 
              size_t value_size);
int cache_set_with_ttl(cache_instance_t* cache, const char* key, 
                       const void* value, size_t value_size, uint64_t ttl_ms);
int cache_get(cache_instance_t* cache, const char* key, void** value, 
              size_t* value_size);
int cache_delete(cache_instance_t* cache, const char* key);
bool cache_exists(cache_instance_t* cache, const char* key);

// Atomic operations
int cache_increment(cache_instance_t* cache, const char* key, int64_t delta, 
                    int64_t* new_value);
int cache_decrement(cache_instance_t* cache, const char* key, int64_t delta, 
                    int64_t* new_value);
int cache_append(cache_instance_t* cache, const char* key, const void* value, 
                 size_t value_size);

// Batch operations
int cache_mget(cache_instance_t* cache, const char** keys, size_t key_count,
               void*** values, size_t** value_sizes);
int cache_mset(cache_instance_t* cache, const char** keys, void** values,
               size_t* value_sizes, size_t count);
int cache_mdelete(cache_instance_t* cache, const char** keys, size_t key_count);

// =============================================================================
// TTL (TIME TO LIVE) MANAGEMENT
// =============================================================================

typedef struct {
    char key[256];
    uint64_t created_at;
    uint64_t ttl_ms;
    uint64_t expires_at;
    bool is_expired;
} ttl_info_t;

// TTL operations
int cache_set_ttl(cache_instance_t* cache, const char* key, uint64_t ttl_ms);
int cache_get_ttl(cache_instance_t* cache, const char* key, uint64_t* ttl_ms);
int cache_persist(cache_instance_t* cache, const char* key); // Remove TTL
int cache_touch(cache_instance_t* cache, const char* key);   // Refresh TTL

// TTL management
int cache_expire_keys(cache_instance_t* cache); // Manually trigger expiration
int cache_get_expiring_keys(cache_instance_t* cache, uint64_t within_ms,
                            char*** keys, size_t* count);

// =============================================================================
// CACHE INVALIDATION STRATEGIES
// =============================================================================

typedef enum {
    INVALIDATION_MANUAL,     // Manual invalidation via API
    INVALIDATION_TTL,        // Time-based expiration
    INVALIDATION_EVENT,      // Event-driven (pub/sub)
    INVALIDATION_VERSION,    // Version-based invalidation
    INVALIDATION_TAG         // Tag-based group invalidation
} invalidation_strategy_t;

typedef struct invalidation_config {
    invalidation_strategy_t strategy;
    uint64_t check_interval_ms;
    bool propagate_to_cluster;
} invalidation_config_t;

// Invalidation API
int cache_invalidate_key(cache_instance_t* cache, const char* key);
int cache_invalidate_pattern(cache_instance_t* cache, const char* pattern);
int cache_invalidate_tag(cache_instance_t* cache, const char* tag);
int cache_invalidate_all(cache_instance_t* cache);

// Tag-based invalidation
int cache_set_with_tags(cache_instance_t* cache, const char* key, 
                        const void* value, size_t value_size,
                        const char** tags, size_t tag_count);
int cache_get_keys_by_tag(cache_instance_t* cache, const char* tag,
                          char*** keys, size_t* count);

// Version-based invalidation
int cache_set_with_version(cache_instance_t* cache, const char* key,
                           const void* value, size_t value_size,
                           uint64_t version);
int cache_get_with_version(cache_instance_t* cache, const char* key,
                           void** value, size_t* value_size, 
                           uint64_t* version);
int cache_invalidate_version(cache_instance_t* cache, uint64_t version);

// =============================================================================
// CACHE STAMPEDE PREVENTION
// =============================================================================

typedef enum {
    STAMPEDE_LOCK,           // Lock-based approach
    STAMPEDE_PROBABILISTIC,  // Probabilistic early expiration
    STAMPEDE_EXTERNAL_LOCK,  // Distributed lock (Redis)
    STAMPEDE_PRECOMPUTE      // Background refresh before expiration
} stampede_prevention_t;

typedef struct {
    stampede_prevention_t method;
    uint64_t lock_timeout_ms;
    double early_expiration_factor; // For probabilistic (0.0-1.0)
    uint64_t refresh_threshold_ms;  // For precompute
} stampede_config_t;

// Stampede prevention
cache_instance_t* cache_with_stampede_prevention(cache_instance_t* cache,
                                                  stampede_config_t* config);
int cache_get_with_lock(cache_instance_t* cache, const char* key,
                        void** value, size_t* value_size,
                        void* (*loader_fn)(const char* key, void* ctx),
                        void* ctx);

// Lock primitives for stampede prevention
typedef struct cache_lock cache_lock_t;

cache_lock_t* cache_lock_acquire(cache_instance_t* cache, const char* key,
                                 uint64_t timeout_ms);
int cache_lock_release(cache_lock_t* lock);
bool cache_lock_try_acquire(cache_instance_t* cache, const char* key,
                            cache_lock_t** lock);

// =============================================================================
// READ-THROUGH & WRITE-THROUGH CACHING
// =============================================================================

// Callback function types
typedef void* (*cache_loader_fn)(const char* key, void* ctx, size_t* size);
typedef int (*cache_writer_fn)(const char* key, const void* value, 
                                size_t size, void* ctx);

typedef struct {
    cache_loader_fn loader;
    cache_writer_fn writer;
    void* loader_ctx;
    void* writer_ctx;
    uint64_t default_ttl_ms;
    bool enable_write_coalescing;
} through_cache_config_t;

// Read-through cache
typedef struct read_through_cache read_through_cache_t;

read_through_cache_t* read_through_cache_create(cache_instance_t* cache,
                                                 cache_loader_fn loader,
                                                 void* loader_ctx);
void read_through_cache_destroy(read_through_cache_t* rt_cache);
int read_through_get(read_through_cache_t* rt_cache, const char* key,
                     void** value, size_t* value_size);

// Write-through cache
typedef struct write_through_cache write_through_cache_t;

write_through_cache_t* write_through_cache_create(cache_instance_t* cache,
                                                   cache_writer_fn writer,
                                                   void* writer_ctx);
void write_through_cache_destroy(write_through_cache_t* wt_cache);
int write_through_set(write_through_cache_t* wt_cache, const char* key,
                      const void* value, size_t value_size);
int write_through_delete(write_through_cache_t* wt_cache, const char* key);

// Write-behind cache (async writes)
typedef struct write_behind_cache write_behind_cache_t;

write_behind_cache_t* write_behind_cache_create(cache_instance_t* cache,
                                                 cache_writer_fn writer,
                                                 void* writer_ctx,
                                                 size_t queue_size);
void write_behind_cache_destroy(write_behind_cache_t* wb_cache);
int write_behind_set(write_behind_cache_t* wb_cache, const char* key,
                     const void* value, size_t value_size);
int write_behind_flush(write_behind_cache_t* wb_cache);

// =============================================================================
// CACHE WARMING & PRELOADING
// =============================================================================

typedef enum {
    WARMUP_ALL,              // Load all data
    WARMUP_POPULAR,          // Load most accessed data
    WARMUP_CRITICAL,         // Load business-critical data
    WARMUP_INCREMENTAL       // Load data incrementally
} warmup_strategy_t;

typedef struct {
    warmup_strategy_t strategy;
    size_t batch_size;
    uint64_t delay_between_batches_ms;
    double max_cpu_usage;
    double max_memory_usage;
} warmup_config_t;

typedef struct {
    uint64_t keys_loaded;
    uint64_t keys_failed;
    uint64_t total_time_ms;
    double memory_used_mb;
} warmup_result_t;

// Cache warming
warmup_result_t* cache_warmup(cache_instance_t* cache, 
                              cache_loader_fn loader,
                              void* loader_ctx,
                              warmup_config_t* config);
int cache_preload_keys(cache_instance_t* cache, const char** keys, 
                       size_t key_count,
                       cache_loader_fn loader, void* loader_ctx);
void warmup_result_destroy(warmup_result_t* result);

// =============================================================================
// CACHE STATISTICS & MONITORING
// =============================================================================

typedef struct {
    uint64_t hits;
    uint64_t misses;
    uint64_t sets;
    uint64_t deletes;
    uint64_t evictions;
    uint64_t expirations;
    double hit_rate;
    double miss_rate;
    size_t current_size;
    size_t max_size;
    double memory_used_mb;
    double memory_max_mb;
    uint64_t total_keys;
    double avg_ttl_ms;
} cache_stats_t;

typedef struct {
    char key[256];
    size_t value_size;
    uint64_t access_count;
    uint64_t last_access_time;
    uint64_t created_at;
    uint64_t ttl_ms;
    bool is_expired;
} cache_key_info_t;

// Statistics
int cache_get_stats(cache_instance_t* cache, cache_stats_t* stats);
int cache_reset_stats(cache_instance_t* cache);
int cache_get_key_info(cache_instance_t* cache, const char* key,
                       cache_key_info_t* info);
int cache_get_all_keys(cache_instance_t* cache, char*** keys, size_t* count);
int cache_get_size(cache_instance_t* cache, size_t* size);

// Performance monitoring
typedef struct {
    char operation[64];
    uint64_t count;
    double total_time_ms;
    double min_time_ms;
    double max_time_ms;
    double avg_time_ms;
    double p95_time_ms;
    double p99_time_ms;
} cache_operation_stats_t;

int cache_get_operation_stats(cache_instance_t* cache,
                              cache_operation_stats_t** stats,
                              size_t* count);

// =============================================================================
// DISTRIBUTED CACHE & REPLICATION
// =============================================================================

typedef enum {
    REPLICATION_MASTER_SLAVE,
    REPLICATION_MASTER_MASTER,
    REPLICATION_CHAIN,
    REPLICATION_TREE
} replication_topology_t;

typedef struct {
    char node_id[64];
    char host[256];
    int port;
    bool is_master;
    bool is_connected;
    uint64_t lag_ms;
} cache_node_info_t;

typedef struct {
    replication_topology_t topology;
    size_t replication_factor;
    uint64_t sync_timeout_ms;
    bool enable_read_from_replicas;
} replication_config_t;

// Distributed cache cluster
cache_cluster_t* cache_cluster_create(replication_config_t* config);
void cache_cluster_destroy(cache_cluster_t* cluster);
int cache_cluster_add_node(cache_cluster_t* cluster, const char* host, int port);
int cache_cluster_remove_node(cache_cluster_t* cluster, const char* node_id);
int cache_cluster_get_nodes(cache_cluster_t* cluster, 
                            cache_node_info_t** nodes, size_t* count);

// Cluster operations
int cache_cluster_set(cache_cluster_t* cluster, const char* key,
                     const void* value, size_t value_size);
int cache_cluster_get(cache_cluster_t* cluster, const char* key,
                     void** value, size_t* value_size);
int cache_cluster_delete(cache_cluster_t* cluster, const char* key);

// =============================================================================
// CACHE CONSISTENCY & CORRECTNESS
// =============================================================================

typedef enum {
    CONSISTENCY_STRONG,      // Synchronous replication, reads always fresh
    CONSISTENCY_EVENTUAL,    // Async replication, may read stale
    CONSISTENCY_BOUNDED      // Stale reads bounded by time
} consistency_level_t;

typedef struct {
    consistency_level_t level;
    uint64_t max_staleness_ms;  // For bounded consistency
    bool enable_versioning;
    bool enable_conflict_resolution;
} consistency_config_t;

// Consistency validation
int cache_validate_consistency(cache_instance_t* cache, const char* key,
                               const void* expected_value, size_t expected_size);
int cache_get_consistency_violations(cache_instance_t* cache,
                                     char*** keys, size_t* count);

// Cache correctness testing
typedef struct {
    uint64_t total_operations;
    uint64_t consistency_violations;
    uint64_t data_loss_events;
    uint64_t stale_reads;
    double avg_staleness_ms;
} cache_correctness_report_t;

cache_correctness_report_t* cache_test_correctness(cache_instance_t* cache,
                                                    uint64_t test_duration_ms);

#endif // CACHE_STRATEGIES_H
