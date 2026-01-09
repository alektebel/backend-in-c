#ifndef NOSQL_H
#define NOSQL_H

#include "common.h"

// ============================================================================
// NoSQL Database Abstraction
// ============================================================================

typedef enum {
    NOSQL_DRIVER_MONGODB,
    NOSQL_DRIVER_REDIS,
    NOSQL_DRIVER_CASSANDRA,
    NOSQL_DRIVER_DYNAMODB
} nosql_driver_t;

typedef enum {
    NOSQL_TYPE_DOCUMENT,  // MongoDB, CouchDB
    NOSQL_TYPE_KEY_VALUE, // Redis, DynamoDB
    NOSQL_TYPE_COLUMN,    // Cassandra, HBase
    NOSQL_TYPE_GRAPH      // Neo4j
} nosql_type_t;

typedef struct nosql_connection nosql_connection_t;
typedef struct nosql_collection nosql_collection_t;
typedef struct nosql_document nosql_document_t;
typedef struct nosql_query nosql_query_t;
typedef struct nosql_cursor nosql_cursor_t;

// ============================================================================
// Document Store (MongoDB-style)
// ============================================================================

// Connection management
nosql_connection_t* nosql_connection_create(nosql_driver_t driver, 
                                            const char* connection_string);
void nosql_connection_destroy(nosql_connection_t* conn);
int nosql_connection_open(nosql_connection_t* conn);
int nosql_connection_close(nosql_connection_t* conn);

// Collection operations
nosql_collection_t* nosql_get_collection(nosql_connection_t* conn, 
                                         const char* database,
                                         const char* collection);
void nosql_collection_destroy(nosql_collection_t* collection);

// Document operations
nosql_document_t* nosql_document_create(void);
void nosql_document_destroy(nosql_document_t* doc);
int nosql_document_set_string(nosql_document_t* doc, const char* key, 
                              const char* value);
int nosql_document_set_int(nosql_document_t* doc, const char* key, int64_t value);
int nosql_document_set_double(nosql_document_t* doc, const char* key, double value);
int nosql_document_set_bool(nosql_document_t* doc, const char* key, int value);
int nosql_document_set_binary(nosql_document_t* doc, const char* key,
                              const void* data, size_t length);
int nosql_document_set_document(nosql_document_t* doc, const char* key,
                               const nosql_document_t* subdoc);
const char* nosql_document_get_string(const nosql_document_t* doc, const char* key);
int64_t nosql_document_get_int(const nosql_document_t* doc, const char* key);
double nosql_document_get_double(const nosql_document_t* doc, const char* key);
int nosql_document_get_bool(const nosql_document_t* doc, const char* key);
char* nosql_document_to_json(const nosql_document_t* doc);
nosql_document_t* nosql_document_from_json(const char* json);

// CRUD operations
int nosql_insert_one(nosql_collection_t* collection, const nosql_document_t* doc);
int nosql_insert_many(nosql_collection_t* collection, const nosql_document_t** docs,
                     size_t count);
nosql_document_t* nosql_find_one(nosql_collection_t* collection, 
                                 const nosql_document_t* filter);
nosql_cursor_t* nosql_find(nosql_collection_t* collection, 
                          const nosql_document_t* filter);
int nosql_update_one(nosql_collection_t* collection, const nosql_document_t* filter,
                    const nosql_document_t* update);
int nosql_update_many(nosql_collection_t* collection, const nosql_document_t* filter,
                     const nosql_document_t* update);
int nosql_delete_one(nosql_collection_t* collection, const nosql_document_t* filter);
int nosql_delete_many(nosql_collection_t* collection, const nosql_document_t* filter);
int64_t nosql_count_documents(nosql_collection_t* collection, 
                              const nosql_document_t* filter);

// Cursor operations
int nosql_cursor_has_next(nosql_cursor_t* cursor);
nosql_document_t* nosql_cursor_next(nosql_cursor_t* cursor);
void nosql_cursor_destroy(nosql_cursor_t* cursor);

// ============================================================================
// Key-Value Store (Redis-style)
// ============================================================================

typedef struct nosql_kv_store nosql_kv_store_t;

nosql_kv_store_t* nosql_kv_create(const char* connection_string);
void nosql_kv_destroy(nosql_kv_store_t* store);

// String operations
int nosql_kv_set(nosql_kv_store_t* store, const char* key, const char* value);
int nosql_kv_set_ex(nosql_kv_store_t* store, const char* key, const char* value,
                    uint64_t ttl_seconds);
char* nosql_kv_get(nosql_kv_store_t* store, const char* key);
int nosql_kv_delete(nosql_kv_store_t* store, const char* key);
int nosql_kv_exists(nosql_kv_store_t* store, const char* key);

// Number operations
int nosql_kv_incr(nosql_kv_store_t* store, const char* key, int64_t* result);
int nosql_kv_decr(nosql_kv_store_t* store, const char* key, int64_t* result);
int nosql_kv_incrby(nosql_kv_store_t* store, const char* key, int64_t increment,
                   int64_t* result);

// List operations
int nosql_kv_lpush(nosql_kv_store_t* store, const char* key, const char* value);
int nosql_kv_rpush(nosql_kv_store_t* store, const char* key, const char* value);
char* nosql_kv_lpop(nosql_kv_store_t* store, const char* key);
char* nosql_kv_rpop(nosql_kv_store_t* store, const char* key);
int64_t nosql_kv_llen(nosql_kv_store_t* store, const char* key);

// Set operations
int nosql_kv_sadd(nosql_kv_store_t* store, const char* key, const char* member);
int nosql_kv_srem(nosql_kv_store_t* store, const char* key, const char* member);
int nosql_kv_sismember(nosql_kv_store_t* store, const char* key, const char* member);
char** nosql_kv_smembers(nosql_kv_store_t* store, const char* key, size_t* count);

// Hash operations
int nosql_kv_hset(nosql_kv_store_t* store, const char* key, const char* field,
                 const char* value);
char* nosql_kv_hget(nosql_kv_store_t* store, const char* key, const char* field);
int nosql_kv_hdel(nosql_kv_store_t* store, const char* key, const char* field);
int nosql_kv_hexists(nosql_kv_store_t* store, const char* key, const char* field);

// Pub/Sub operations
int nosql_kv_publish(nosql_kv_store_t* store, const char* channel, 
                    const char* message);
int nosql_kv_subscribe(nosql_kv_store_t* store, const char* channel,
                      void (*callback)(const char* channel, const char* message,
                                      void* user_data),
                      void* user_data);

// ============================================================================
// Indexing and Query Optimization
// ============================================================================

typedef struct nosql_index nosql_index_t;

typedef enum {
    NOSQL_INDEX_ASCENDING,
    NOSQL_INDEX_DESCENDING,
    NOSQL_INDEX_TEXT,
    NOSQL_INDEX_GEOSPATIAL,
    NOSQL_INDEX_HASHED
} nosql_index_type_t;

int nosql_create_index(nosql_collection_t* collection, const char* field,
                      nosql_index_type_t type);
int nosql_create_compound_index(nosql_collection_t* collection, 
                               const char** fields, nosql_index_type_t* types,
                               size_t field_count);
int nosql_drop_index(nosql_collection_t* collection, const char* index_name);
char** nosql_list_indexes(nosql_collection_t* collection, size_t* count);

// ============================================================================
// Aggregation Pipeline
// ============================================================================

typedef struct nosql_pipeline nosql_pipeline_t;

nosql_pipeline_t* nosql_pipeline_create(void);
void nosql_pipeline_destroy(nosql_pipeline_t* pipeline);
int nosql_pipeline_match(nosql_pipeline_t* pipeline, const nosql_document_t* filter);
int nosql_pipeline_group(nosql_pipeline_t* pipeline, const char* group_by);
int nosql_pipeline_sort(nosql_pipeline_t* pipeline, const char* field, int ascending);
int nosql_pipeline_limit(nosql_pipeline_t* pipeline, int limit);
int nosql_pipeline_skip(nosql_pipeline_t* pipeline, int skip);
nosql_cursor_t* nosql_aggregate(nosql_collection_t* collection, 
                               const nosql_pipeline_t* pipeline);

// ============================================================================
// Replication and Sharding
// ============================================================================

typedef struct nosql_replica_set nosql_replica_set_t;

nosql_replica_set_t* nosql_replica_set_create(const char* name);
void nosql_replica_set_destroy(nosql_replica_set_t* replica_set);
int nosql_replica_set_add_member(nosql_replica_set_t* replica_set, 
                                const char* host, int port);
int nosql_replica_set_remove_member(nosql_replica_set_t* replica_set,
                                   const char* host, int port);

// Sharding
typedef struct nosql_shard_cluster nosql_shard_cluster_t;

nosql_shard_cluster_t* nosql_shard_cluster_create(void);
void nosql_shard_cluster_destroy(nosql_shard_cluster_t* cluster);
int nosql_shard_enable(nosql_collection_t* collection, const char* shard_key);
int nosql_shard_add_server(nosql_shard_cluster_t* cluster, const char* host, int port);

#endif // NOSQL_H
