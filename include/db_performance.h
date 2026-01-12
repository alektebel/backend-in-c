#ifndef DB_PERFORMANCE_H
#define DB_PERFORMANCE_H

#include "common.h"
#include <stdint.h>
#include <stdbool.h>

// =============================================================================
// DATABASE PERFORMANCE & QUERY OPTIMIZATION
// =============================================================================
// This module provides templates for:
// - Indexing strategies (B-tree, Hash, Bitmap, Full-text)
// - Query optimization (EXPLAIN plans, cost estimation)
// - N+1 query problem detection and solutions
// - Pagination strategies (OFFSET vs Cursor-based)
// - Connection pooling and management

// Forward declarations
typedef struct db_connection db_connection_t;
typedef struct db_connection_pool db_connection_pool_t;
typedef struct db_index db_index_t;
typedef struct query_plan query_plan_t;
typedef struct query_optimizer query_optimizer_t;

// =============================================================================
// INDEX TYPES & STRATEGIES
// =============================================================================

typedef enum {
    INDEX_TYPE_BTREE,        // Balanced tree index (good for range queries)
    INDEX_TYPE_HASH,         // Hash index (good for equality lookups)
    INDEX_TYPE_BITMAP,       // Bitmap index (good for low-cardinality columns)
    INDEX_TYPE_FULLTEXT,     // Full-text search index
    INDEX_TYPE_SPATIAL,      // Spatial/GIS index (R-tree)
    INDEX_TYPE_COMPOSITE     // Multi-column index
} index_type_t;

typedef enum {
    INDEX_STATUS_BUILDING,
    INDEX_STATUS_READY,
    INDEX_STATUS_INVALID,
    INDEX_STATUS_DISABLED
} index_status_t;

typedef struct {
    char name[128];
    index_type_t type;
    index_status_t status;
    char table_name[128];
    char** column_names;
    size_t column_count;
    size_t entries;
    size_t size_bytes;
    bool is_unique;
    bool is_clustered;
} index_info_t;

// Index management
db_index_t* db_index_create(const char* name, index_type_t type, 
                            const char* table, const char** columns, 
                            size_t column_count);
void db_index_destroy(db_index_t* index);
int db_index_build(db_index_t* index);
int db_index_rebuild(db_index_t* index);
int db_index_drop(db_index_t* index);
int db_index_get_info(db_index_t* index, index_info_t* info);

// Index usage analysis
typedef struct {
    char index_name[128];
    uint64_t scans;
    uint64_t lookups;
    uint64_t inserts;
    uint64_t updates;
    uint64_t deletes;
    double selectivity;       // Ratio of rows selected vs total rows
    double hit_rate;
} index_usage_stats_t;

int db_index_get_usage_stats(db_index_t* index, index_usage_stats_t* stats);
int db_recommend_indexes(const char* query, index_info_t** recommendations, 
                         size_t* count);

// =============================================================================
// QUERY PLANS & OPTIMIZATION
// =============================================================================

typedef enum {
    PLAN_NODE_SCAN,          // Full table scan
    PLAN_NODE_INDEX_SCAN,    // Index scan
    PLAN_NODE_INDEX_SEEK,    // Index seek (with predicate)
    PLAN_NODE_NESTED_LOOP,   // Nested loop join
    PLAN_NODE_HASH_JOIN,     // Hash join
    PLAN_NODE_MERGE_JOIN,    // Merge join
    PLAN_NODE_SORT,          // Sort operation
    PLAN_NODE_AGGREGATE,     // Aggregation (GROUP BY)
    PLAN_NODE_FILTER         // WHERE clause filter
} plan_node_type_t;

typedef struct plan_node {
    plan_node_type_t type;
    char operation[256];
    double estimated_cost;
    uint64_t estimated_rows;
    double actual_cost;
    uint64_t actual_rows;
    char** used_indexes;
    size_t index_count;
    struct plan_node* left_child;
    struct plan_node* right_child;
} plan_node_t;

typedef struct query_plan {
    char query[4096];
    plan_node_t* root;
    double total_cost;
    uint64_t total_rows;
    double execution_time_ms;
    bool uses_index;
    char warnings[1024];
} query_plan_t;

// Query plan analysis
query_plan_t* db_explain_query(db_connection_t* conn, const char* query);
void db_query_plan_destroy(query_plan_t* plan);
int db_query_plan_print(query_plan_t* plan);
int db_query_plan_to_json(query_plan_t* plan, char* buffer, size_t size);

// Query optimizer
query_optimizer_t* db_optimizer_create(void);
void db_optimizer_destroy(query_optimizer_t* optimizer);
int db_optimizer_set_option(query_optimizer_t* opt, const char* key, 
                            const char* value);
char* db_optimizer_rewrite_query(query_optimizer_t* opt, const char* query);

// =============================================================================
// N+1 QUERY PROBLEM DETECTION & SOLUTIONS
// =============================================================================

typedef struct {
    char parent_query[1024];
    uint64_t parent_executions;
    char child_query[1024];
    uint64_t child_executions;
    double total_time_ms;
    bool is_n_plus_one;
} n_plus_one_detection_t;

typedef struct query_tracker query_tracker_t;

// N+1 detection
query_tracker_t* db_query_tracker_create(void);
void db_query_tracker_destroy(query_tracker_t* tracker);
int db_query_tracker_record(query_tracker_t* tracker, const char* query, 
                            double execution_time_ms);
int db_query_tracker_analyze(query_tracker_t* tracker, 
                             n_plus_one_detection_t** results, size_t* count);

// N+1 solution strategies
typedef enum {
    N_PLUS_ONE_FIX_EAGER_LOADING,      // Load all data in single query
    N_PLUS_ONE_FIX_BATCHING,           // Batch multiple queries
    N_PLUS_ONE_FIX_CACHING,            // Cache repeated queries
    N_PLUS_ONE_FIX_SUBQUERY,           // Use subquery/JOIN
    N_PLUS_ONE_FIX_DATALOADER          // DataLoader pattern
} n_plus_one_fix_strategy_t;

char* db_suggest_n_plus_one_fix(n_plus_one_detection_t* detection, 
                                 n_plus_one_fix_strategy_t strategy);

// =============================================================================
// PAGINATION STRATEGIES
// =============================================================================

typedef enum {
    PAGINATION_OFFSET,       // OFFSET/LIMIT (simple but slow for large offsets)
    PAGINATION_CURSOR,       // Cursor-based (efficient, consistent)
    PAGINATION_KEYSET        // Keyset/seek method (best for large datasets)
} pagination_strategy_t;

typedef struct {
    pagination_strategy_t strategy;
    uint64_t page_size;
    uint64_t current_page;
    uint64_t total_items;
    uint64_t total_pages;
    char cursor[256];        // For cursor-based pagination
    char* sort_columns[8];
    size_t sort_column_count;
} pagination_config_t;

typedef struct {
    void** items;
    size_t item_count;
    char next_cursor[256];
    char prev_cursor[256];
    bool has_next;
    bool has_prev;
    double query_time_ms;
} pagination_result_t;

// Pagination API
pagination_config_t* db_pagination_config_create(pagination_strategy_t strategy, 
                                                  uint64_t page_size);
void db_pagination_config_destroy(pagination_config_t* config);

pagination_result_t* db_paginate_offset(db_connection_t* conn, 
                                        const char* query, 
                                        pagination_config_t* config);
pagination_result_t* db_paginate_cursor(db_connection_t* conn, 
                                        const char* query, 
                                        pagination_config_t* config);
pagination_result_t* db_paginate_keyset(db_connection_t* conn, 
                                        const char* query, 
                                        pagination_config_t* config);
void db_pagination_result_destroy(pagination_result_t* result);

// Pagination comparison & benchmarking
typedef struct {
    pagination_strategy_t strategy;
    uint64_t page_number;
    double query_time_ms;
    uint64_t rows_scanned;
    bool uses_index;
} pagination_benchmark_t;

int db_compare_pagination_strategies(db_connection_t* conn, const char* query,
                                     pagination_benchmark_t** results, 
                                     size_t* count);

// =============================================================================
// CONNECTION POOLING
// =============================================================================

typedef enum {
    CONN_STATE_IDLE,
    CONN_STATE_IN_USE,
    CONN_STATE_INVALID,
    CONN_STATE_CLOSED
} connection_state_t;

typedef struct {
    char id[64];
    connection_state_t state;
    uint64_t created_at;
    uint64_t last_used_at;
    uint64_t use_count;
    char current_query[512];
} connection_info_t;

typedef struct {
    size_t min_connections;
    size_t max_connections;
    size_t idle_timeout_ms;
    size_t max_lifetime_ms;
    size_t connection_timeout_ms;
    bool validate_on_borrow;
    bool validate_on_return;
} pool_config_t;

typedef struct {
    size_t total_connections;
    size_t active_connections;
    size_t idle_connections;
    size_t wait_count;
    double avg_wait_time_ms;
    double avg_use_time_ms;
    uint64_t total_created;
    uint64_t total_destroyed;
    uint64_t total_borrowed;
    uint64_t total_returned;
} pool_stats_t;

// Connection pool management
db_connection_pool_t* db_pool_create(const char* connection_string, 
                                     pool_config_t* config);
void db_pool_destroy(db_connection_pool_t* pool);

db_connection_t* db_pool_acquire(db_connection_pool_t* pool);
int db_pool_release(db_connection_pool_t* pool, db_connection_t* conn);
int db_pool_validate_connection(db_connection_t* conn);

int db_pool_get_stats(db_connection_pool_t* pool, pool_stats_t* stats);
int db_pool_get_connection_info(db_connection_pool_t* pool, 
                                connection_info_t** connections, size_t* count);

// Pool maintenance
int db_pool_shrink(db_connection_pool_t* pool);
int db_pool_grow(db_connection_pool_t* pool, size_t count);
int db_pool_clear_idle(db_connection_pool_t* pool);
int db_pool_health_check(db_connection_pool_t* pool);

// =============================================================================
// QUERY PERFORMANCE MONITORING
// =============================================================================

typedef struct {
    char query[2048];
    char query_hash[64];     // Hash for grouping similar queries
    uint64_t execution_count;
    double total_time_ms;
    double min_time_ms;
    double max_time_ms;
    double avg_time_ms;
    double p95_time_ms;
    double p99_time_ms;
    uint64_t rows_examined;
    uint64_t rows_sent;
    uint64_t tmp_tables;
    bool uses_filesort;
    bool uses_temporary;
} query_stats_t;

typedef struct query_monitor query_monitor_t;

// Query monitoring
query_monitor_t* db_query_monitor_create(void);
void db_query_monitor_destroy(query_monitor_t* monitor);
int db_query_monitor_record(query_monitor_t* monitor, const char* query, 
                            double execution_time_ms, uint64_t rows_examined);
int db_query_monitor_get_slow_queries(query_monitor_t* monitor, 
                                      double threshold_ms, 
                                      query_stats_t** queries, size_t* count);
int db_query_monitor_get_top_queries(query_monitor_t* monitor, size_t limit,
                                     query_stats_t** queries, size_t* count);
int db_query_monitor_reset(query_monitor_t* monitor);

// =============================================================================
// BULK OPERATIONS & BATCH PROCESSING
// =============================================================================

typedef struct {
    size_t batch_size;
    bool use_transaction;
    bool ignore_errors;
    bool return_inserted_ids;
} bulk_insert_config_t;

typedef struct {
    uint64_t rows_affected;
    uint64_t rows_failed;
    double execution_time_ms;
    char** error_messages;
    size_t error_count;
} bulk_operation_result_t;

// Bulk operations for performance
bulk_operation_result_t* db_bulk_insert(db_connection_t* conn, 
                                        const char* table,
                                        void** rows, size_t row_count,
                                        bulk_insert_config_t* config);
bulk_operation_result_t* db_bulk_update(db_connection_t* conn,
                                        const char* query,
                                        void** params, size_t param_count);
bulk_operation_result_t* db_bulk_delete(db_connection_t* conn,
                                        const char* table,
                                        const char** ids, size_t id_count);
void db_bulk_operation_result_destroy(bulk_operation_result_t* result);

#endif // DB_PERFORMANCE_H
