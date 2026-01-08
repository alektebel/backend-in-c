#ifndef DATABASE_H
#define DATABASE_H

#include "common.h"
#include <pthread.h>

// Database types
typedef struct database database_t;
typedef struct transaction transaction_t;

// Index types
typedef enum {
    INDEX_BTREE,
    INDEX_HASH
} index_type_t;

// Transaction isolation levels
typedef enum {
    ISOLATION_READ_UNCOMMITTED,
    ISOLATION_READ_COMMITTED,
    ISOLATION_REPEATABLE_READ,
    ISOLATION_SERIALIZABLE
} isolation_level_t;

// Database functions
database_t* database_create(const char* data_dir);
void database_destroy(database_t* db);
int database_open(database_t* db);
int database_close(database_t* db);

// Key-value operations
int database_put(database_t* db, const char* key, const void* value, size_t value_size);
int database_get(database_t* db, const char* key, void** value, size_t* value_size);
int database_delete(database_t* db, const char* key);
int database_exists(database_t* db, const char* key);

// Transaction support
transaction_t* database_begin_transaction(database_t* db, isolation_level_t level);
int transaction_commit(transaction_t* txn);
int transaction_rollback(transaction_t* txn);
int transaction_put(transaction_t* txn, const char* key, const void* value, size_t value_size);
int transaction_get(transaction_t* txn, const char* key, void** value, size_t* value_size);
int transaction_delete(transaction_t* txn, const char* key);

// Index operations
int database_create_index(database_t* db, const char* index_name, index_type_t type);
int database_drop_index(database_t* db, const char* index_name);

// WAL operations
int database_checkpoint(database_t* db);
int database_recover(database_t* db);

// Compaction
int database_compact(database_t* db);

// Statistics
typedef struct {
    size_t num_keys;
    size_t total_size;
    size_t wal_size;
    size_t num_transactions;
} database_stats_t;

int database_get_stats(database_t* db, database_stats_t* stats);

#endif // DATABASE_H
