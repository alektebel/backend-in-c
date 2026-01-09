#ifndef SCALING_H
#define SCALING_H

#include "common.h"

// ============================================================================
// Vertical Scaling (Scale Up/Down)
// ============================================================================

typedef struct resource_monitor resource_monitor_t;

typedef struct {
    double cpu_usage_percent;
    uint64_t memory_used_bytes;
    uint64_t memory_total_bytes;
    uint64_t disk_used_bytes;
    uint64_t disk_total_bytes;
    uint64_t network_in_bytes_per_sec;
    uint64_t network_out_bytes_per_sec;
} resource_stats_t;

resource_monitor_t* resource_monitor_create(void);
void resource_monitor_destroy(resource_monitor_t* monitor);
int resource_monitor_get_stats(resource_monitor_t* monitor, resource_stats_t* stats);
int resource_monitor_start(resource_monitor_t* monitor, uint64_t interval_ms);
int resource_monitor_stop(resource_monitor_t* monitor);

// Auto-scaling based on metrics
typedef struct autoscaler autoscaler_t;

typedef struct {
    double cpu_threshold_percent;
    double memory_threshold_percent;
    int scale_up_threshold;
    int scale_down_threshold;
    uint64_t cooldown_period_ms;
} autoscale_config_t;

autoscaler_t* autoscaler_create(const autoscale_config_t* config);
void autoscaler_destroy(autoscaler_t* scaler);
int autoscaler_attach_monitor(autoscaler_t* scaler, resource_monitor_t* monitor);
int autoscaler_set_callback(autoscaler_t* scaler,
                           void (*on_scale_up)(int level, void*),
                           void (*on_scale_down)(int level, void*),
                           void* user_data);
int autoscaler_start(autoscaler_t* scaler);
int autoscaler_stop(autoscaler_t* scaler);

// ============================================================================
// Horizontal Scaling (Scale Out/In)
// ============================================================================

typedef struct cluster_manager cluster_manager_t;
typedef struct cluster_node cluster_node_t;

typedef enum {
    NODE_STATE_JOINING,
    NODE_STATE_ACTIVE,
    NODE_STATE_LEAVING,
    NODE_STATE_DOWN
} node_state_t;

// Cluster management
cluster_manager_t* cluster_manager_create(const char* cluster_name);
void cluster_manager_destroy(cluster_manager_t* manager);
int cluster_manager_join(cluster_manager_t* manager, const char* host, int port);
int cluster_manager_leave(cluster_manager_t* manager);
cluster_node_t** cluster_manager_get_nodes(cluster_manager_t* manager, size_t* count);
int cluster_manager_broadcast(cluster_manager_t* manager, const void* data, 
                              size_t data_len);

// Node operations
const char* cluster_node_get_id(const cluster_node_t* node);
const char* cluster_node_get_host(const cluster_node_t* node);
int cluster_node_get_port(const cluster_node_t* node);
node_state_t cluster_node_get_state(const cluster_node_t* node);

// ============================================================================
// Database Sharding
// ============================================================================

typedef struct shard_manager shard_manager_t;
typedef struct shard shard_t;

typedef enum {
    SHARD_STRATEGY_HASH,
    SHARD_STRATEGY_RANGE,
    SHARD_STRATEGY_DIRECTORY,
    SHARD_STRATEGY_CONSISTENT_HASH
} shard_strategy_t;

// Shard manager operations
shard_manager_t* shard_manager_create(shard_strategy_t strategy);
void shard_manager_destroy(shard_manager_t* manager);
int shard_manager_add_shard(shard_manager_t* manager, const char* shard_id,
                           const char* host, int port);
int shard_manager_remove_shard(shard_manager_t* manager, const char* shard_id);
shard_t* shard_manager_get_shard_for_key(shard_manager_t* manager, const char* key);
int shard_manager_rebalance(shard_manager_t* manager);

// Shard operations
const char* shard_get_id(const shard_t* shard);
const char* shard_get_host(const shard_t* shard);
int shard_get_port(const shard_t* shard);

// ============================================================================
// Database Replication
// ============================================================================

typedef enum {
    REPLICATION_MASTER_SLAVE,
    REPLICATION_MASTER_MASTER,
    REPLICATION_QUORUM
} replication_type_t;

typedef struct replication_manager replication_manager_t;
typedef struct replica replica_t;

// Replication manager operations
replication_manager_t* replication_manager_create(replication_type_t type);
void replication_manager_destroy(replication_manager_t* manager);
int replication_manager_add_replica(replication_manager_t* manager,
                                   const char* host, int port, int is_master);
int replication_manager_remove_replica(replication_manager_t* manager,
                                      const char* replica_id);
int replication_manager_promote_to_master(replication_manager_t* manager,
                                         const char* replica_id);
replica_t** replication_manager_get_replicas(replication_manager_t* manager,
                                            size_t* count);

// Replica operations
const char* replica_get_id(const replica_t* replica);
const char* replica_get_host(const replica_t* replica);
int replica_get_port(const replica_t* replica);
int replica_is_master(const replica_t* replica);
uint64_t replica_get_replication_lag(const replica_t* replica);

// ============================================================================
// Consistent Hashing (for distributed systems)
// ============================================================================

typedef struct consistent_hash consistent_hash_t;

consistent_hash_t* consistent_hash_create(int virtual_nodes);
void consistent_hash_destroy(consistent_hash_t* hash);
int consistent_hash_add_node(consistent_hash_t* hash, const char* node_id);
int consistent_hash_remove_node(consistent_hash_t* hash, const char* node_id);
const char* consistent_hash_get_node(consistent_hash_t* hash, const char* key);
char** consistent_hash_get_nodes(consistent_hash_t* hash, const char* key,
                                size_t n, size_t* count);

// ============================================================================
// Data Partitioning
// ============================================================================

typedef struct partition_manager partition_manager_t;

typedef enum {
    PARTITION_HORIZONTAL,  // Row-based partitioning
    PARTITION_VERTICAL,    // Column-based partitioning
    PARTITION_FUNCTIONAL   // Feature-based partitioning
} partition_type_t;

partition_manager_t* partition_manager_create(partition_type_t type);
void partition_manager_destroy(partition_manager_t* manager);
int partition_manager_add_partition(partition_manager_t* manager,
                                   const char* partition_id,
                                   const char* criteria);
int partition_manager_get_partition_for_data(partition_manager_t* manager,
                                            const void* data,
                                            char** partition_id);

// ============================================================================
// Read-Write Splitting
// ============================================================================

typedef struct rw_splitter rw_splitter_t;

rw_splitter_t* rw_splitter_create(void);
void rw_splitter_destroy(rw_splitter_t* splitter);
int rw_splitter_set_write_endpoint(rw_splitter_t* splitter, const char* host, int port);
int rw_splitter_add_read_endpoint(rw_splitter_t* splitter, const char* host, int port);
int rw_splitter_route_query(rw_splitter_t* splitter, const char* query,
                            char** host, int* port);

#endif // SCALING_H
