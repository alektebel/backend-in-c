#ifndef LOGGING_H
#define LOGGING_H

#include "common.h"
#include <stdarg.h>

// ============================================================================
// Logging Levels
// ============================================================================

typedef enum {
    LOG_LEVEL_TRACE,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_FATAL
} log_level_t;

// ============================================================================
// Structured Logging
// ============================================================================

typedef struct logger logger_t;
typedef struct log_context log_context_t;
typedef struct log_field log_field_t;

// Logger operations
logger_t* logger_create(const char* name);
void logger_destroy(logger_t* logger);
int logger_set_level(logger_t* logger, log_level_t level);
log_level_t logger_get_level(const logger_t* logger);

// Basic logging
void logger_trace(logger_t* logger, const char* format, ...);
void logger_debug(logger_t* logger, const char* format, ...);
void logger_info(logger_t* logger, const char* format, ...);
void logger_warn(logger_t* logger, const char* format, ...);
void logger_error(logger_t* logger, const char* format, ...);
void logger_fatal(logger_t* logger, const char* format, ...);

// Structured logging with fields
log_context_t* log_context_create(void);
void log_context_destroy(log_context_t* ctx);
int log_context_add_string(log_context_t* ctx, const char* key, const char* value);
int log_context_add_int(log_context_t* ctx, const char* key, int64_t value);
int log_context_add_double(log_context_t* ctx, const char* key, double value);
int log_context_add_bool(log_context_t* ctx, const char* key, int value);
void logger_log_with_context(logger_t* logger, log_level_t level,
                            const char* message, log_context_t* ctx);

// ============================================================================
// Log Formatters
// ============================================================================

typedef enum {
    LOG_FORMAT_TEXT,
    LOG_FORMAT_JSON,
    LOG_FORMAT_LOGFMT,
    LOG_FORMAT_CUSTOM
} log_format_t;

typedef struct log_formatter log_formatter_t;

log_formatter_t* log_formatter_create(log_format_t format);
void log_formatter_destroy(log_formatter_t* formatter);
char* log_formatter_format(log_formatter_t* formatter, log_level_t level,
                          const char* logger_name, const char* message,
                          log_context_t* ctx);

// ============================================================================
// Log Appenders/Handlers
// ============================================================================

typedef enum {
    LOG_APPENDER_CONSOLE,
    LOG_APPENDER_FILE,
    LOG_APPENDER_SYSLOG,
    LOG_APPENDER_NETWORK,
    LOG_APPENDER_CUSTOM
} log_appender_type_t;

typedef struct log_appender log_appender_t;

// Console appender
log_appender_t* log_appender_console_create(void);

// File appender
typedef struct {
    const char* filename;
    uint64_t max_file_size;
    int max_backup_files;
    int append;
} log_file_config_t;

log_appender_t* log_appender_file_create(const log_file_config_t* config);

// Syslog appender
log_appender_t* log_appender_syslog_create(const char* ident, int facility);

// Network appender (for log aggregation)
log_appender_t* log_appender_network_create(const char* host, int port,
                                           const char* protocol);

// Appender operations
void log_appender_destroy(log_appender_t* appender);
int log_appender_write(log_appender_t* appender, const char* formatted_log);
int logger_add_appender(logger_t* logger, log_appender_t* appender);
int logger_remove_appender(logger_t* logger, log_appender_t* appender);

// ============================================================================
// Log Rotation
// ============================================================================

typedef enum {
    LOG_ROTATION_SIZE,
    LOG_ROTATION_TIME,
    LOG_ROTATION_DAILY,
    LOG_ROTATION_WEEKLY
} log_rotation_type_t;

typedef struct log_rotator log_rotator_t;

log_rotator_t* log_rotator_create(log_rotation_type_t type, uint64_t threshold);
void log_rotator_destroy(log_rotator_t* rotator);
int log_rotator_attach(log_rotator_t* rotator, const char* log_file);
int log_rotator_check_and_rotate(log_rotator_t* rotator);

// ============================================================================
// Log Filtering
// ============================================================================

typedef struct log_filter log_filter_t;

log_filter_t* log_filter_create(void);
void log_filter_destroy(log_filter_t* filter);
int log_filter_add_rule(log_filter_t* filter, const char* logger_name,
                       log_level_t min_level);
int log_filter_should_log(log_filter_t* filter, const char* logger_name,
                         log_level_t level);
int logger_add_filter(logger_t* logger, log_filter_t* filter);

// ============================================================================
// Request ID and Correlation
// ============================================================================

typedef struct correlation_context correlation_context_t;

correlation_context_t* correlation_context_create(const char* request_id);
void correlation_context_destroy(correlation_context_t* ctx);
const char* correlation_context_get_request_id(const correlation_context_t* ctx);
int correlation_context_set_user_id(correlation_context_t* ctx, const char* user_id);
int correlation_context_set_session_id(correlation_context_t* ctx, 
                                       const char* session_id);
int logger_set_correlation_context(logger_t* logger, correlation_context_t* ctx);

// ============================================================================
// Performance Logging
// ============================================================================

typedef struct perf_logger perf_logger_t;

perf_logger_t* perf_logger_create(logger_t* logger);
void perf_logger_destroy(perf_logger_t* perf);
uint64_t perf_logger_start(perf_logger_t* perf, const char* operation);
void perf_logger_end(perf_logger_t* perf, uint64_t start_time, const char* operation);
void perf_logger_log_duration(perf_logger_t* perf, const char* operation,
                             uint64_t duration_ms);

// ============================================================================
// Audit Logging
// ============================================================================

typedef struct audit_logger audit_logger_t;

typedef struct {
    const char* user_id;
    const char* action;
    const char* resource;
    const char* result;
    log_context_t* additional_data;
} audit_event_t;

audit_logger_t* audit_logger_create(logger_t* logger);
void audit_logger_destroy(audit_logger_t* audit);
int audit_logger_log_event(audit_logger_t* audit, const audit_event_t* event);

// ============================================================================
// Global Logger Management
// ============================================================================

logger_t* logger_get_global(void);
void logger_set_global(logger_t* logger);
logger_t* logger_get_or_create(const char* name);

// Convenience macros for global logger
#define LOG_TRACE(...) logger_trace(logger_get_global(), __VA_ARGS__)
#define LOG_DEBUG(...) logger_debug(logger_get_global(), __VA_ARGS__)
#define LOG_INFO(...) logger_info(logger_get_global(), __VA_ARGS__)
#define LOG_WARN(...) logger_warn(logger_get_global(), __VA_ARGS__)
#define LOG_ERROR(...) logger_error(logger_get_global(), __VA_ARGS__)
#define LOG_FATAL(...) logger_fatal(logger_get_global(), __VA_ARGS__)

#endif // LOGGING_H
