#ifndef SQL_H
#define SQL_H

#include "common.h"

// ============================================================================
// SQL Database Abstraction Layer
// ============================================================================

typedef enum {
    SQL_DRIVER_SQLITE,
    SQL_DRIVER_MYSQL,
    SQL_DRIVER_POSTGRESQL,
    SQL_DRIVER_MSSQL
} sql_driver_t;

typedef enum {
    SQL_TYPE_NULL,
    SQL_TYPE_INTEGER,
    SQL_TYPE_REAL,
    SQL_TYPE_TEXT,
    SQL_TYPE_BLOB
} sql_data_type_t;

typedef struct sql_connection sql_connection_t;
typedef struct sql_statement sql_statement_t;
typedef struct sql_result sql_result_t;
typedef struct sql_row sql_row_t;

// Connection management
sql_connection_t* sql_connection_create(sql_driver_t driver, const char* connection_string);
void sql_connection_destroy(sql_connection_t* conn);
int sql_connection_open(sql_connection_t* conn);
int sql_connection_close(sql_connection_t* conn);
int sql_connection_ping(sql_connection_t* conn);
const char* sql_connection_get_error(const sql_connection_t* conn);

// Transaction management
int sql_begin_transaction(sql_connection_t* conn);
int sql_commit_transaction(sql_connection_t* conn);
int sql_rollback_transaction(sql_connection_t* conn);
int sql_set_isolation_level(sql_connection_t* conn, const char* level);

// Prepared statements
sql_statement_t* sql_statement_prepare(sql_connection_t* conn, const char* query);
void sql_statement_destroy(sql_statement_t* stmt);
int sql_statement_bind_int(sql_statement_t* stmt, int index, int64_t value);
int sql_statement_bind_double(sql_statement_t* stmt, int index, double value);
int sql_statement_bind_text(sql_statement_t* stmt, int index, const char* value);
int sql_statement_bind_blob(sql_statement_t* stmt, int index, 
                           const void* value, size_t length);
int sql_statement_bind_null(sql_statement_t* stmt, int index);
sql_result_t* sql_statement_execute(sql_statement_t* stmt);
int sql_statement_reset(sql_statement_t* stmt);

// Query execution
sql_result_t* sql_execute_query(sql_connection_t* conn, const char* query);
int sql_execute_non_query(sql_connection_t* conn, const char* query);

// Result handling
void sql_result_destroy(sql_result_t* result);
int sql_result_has_rows(const sql_result_t* result);
size_t sql_result_get_row_count(const sql_result_t* result);
size_t sql_result_get_column_count(const sql_result_t* result);
const char* sql_result_get_column_name(const sql_result_t* result, int column);
sql_row_t* sql_result_fetch_row(sql_result_t* result);

// Row data access
int64_t sql_row_get_int(const sql_row_t* row, int column);
double sql_row_get_double(const sql_row_t* row, int column);
const char* sql_row_get_text(const sql_row_t* row, int column);
const void* sql_row_get_blob(const sql_row_t* row, int column, size_t* length);
int sql_row_is_null(const sql_row_t* row, int column);
sql_data_type_t sql_row_get_type(const sql_row_t* row, int column);
void sql_row_destroy(sql_row_t* row);

// ============================================================================
// Connection Pooling
// ============================================================================

typedef struct sql_pool sql_pool_t;

typedef struct {
    int min_connections;
    int max_connections;
    uint64_t connection_timeout_ms;
    uint64_t idle_timeout_ms;
    int test_on_borrow;
} sql_pool_config_t;

sql_pool_t* sql_pool_create(sql_driver_t driver, const char* connection_string,
                            const sql_pool_config_t* config);
void sql_pool_destroy(sql_pool_t* pool);
sql_connection_t* sql_pool_acquire(sql_pool_t* pool);
int sql_pool_release(sql_pool_t* pool, sql_connection_t* conn);
size_t sql_pool_get_active_count(const sql_pool_t* pool);
size_t sql_pool_get_idle_count(const sql_pool_t* pool);

// ============================================================================
// ACID Properties Support
// ============================================================================

typedef enum {
    ISOLATION_READ_UNCOMMITTED,
    ISOLATION_READ_COMMITTED,
    ISOLATION_REPEATABLE_READ,
    ISOLATION_SERIALIZABLE
} sql_isolation_level_t;

int sql_set_autocommit(sql_connection_t* conn, int enable);
int sql_savepoint(sql_connection_t* conn, const char* name);
int sql_rollback_to_savepoint(sql_connection_t* conn, const char* name);
int sql_release_savepoint(sql_connection_t* conn, const char* name);

// ============================================================================
// Query Builder
// ============================================================================

typedef struct sql_query_builder sql_query_builder_t;

sql_query_builder_t* sql_query_builder_create(void);
void sql_query_builder_destroy(sql_query_builder_t* builder);
sql_query_builder_t* sql_query_select(sql_query_builder_t* builder, 
                                     const char** columns, size_t column_count);
sql_query_builder_t* sql_query_from(sql_query_builder_t* builder, const char* table);
sql_query_builder_t* sql_query_where(sql_query_builder_t* builder, const char* condition);
sql_query_builder_t* sql_query_join(sql_query_builder_t* builder, const char* table,
                                   const char* on_condition);
sql_query_builder_t* sql_query_order_by(sql_query_builder_t* builder, 
                                       const char* column, int ascending);
sql_query_builder_t* sql_query_limit(sql_query_builder_t* builder, int limit);
sql_query_builder_t* sql_query_offset(sql_query_builder_t* builder, int offset);
char* sql_query_build(const sql_query_builder_t* builder);

// ============================================================================
// Migration System
// ============================================================================

typedef struct sql_migration sql_migration_t;
typedef struct sql_migrator sql_migrator_t;

sql_migrator_t* sql_migrator_create(sql_connection_t* conn);
void sql_migrator_destroy(sql_migrator_t* migrator);
int sql_migrator_add_migration(sql_migrator_t* migrator, const char* version,
                               const char* up_sql, const char* down_sql);
int sql_migrator_migrate_up(sql_migrator_t* migrator);
int sql_migrator_migrate_down(sql_migrator_t* migrator, int steps);
int sql_migrator_get_current_version(sql_migrator_t* migrator, char** version);

#endif // SQL_H
