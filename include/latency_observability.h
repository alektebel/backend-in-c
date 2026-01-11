#ifndef LATENCY_OBSERVABILITY_H
#define LATENCY_OBSERVABILITY_H

#include "common.h"
#include <stdint.h>
#include <stdbool.h>

// =============================================================================
// LATENCY BUDGETING & OBSERVABILITY
// =============================================================================
// This module provides templates for:
// - P95/P99 latency tracking and analysis
// - Tail latency detection and mitigation
// - Timeout management
// - Retry policies with backoff
// - Circuit breakers
// - Distributed tracing
// - Bottleneck analysis and optimization

// Forward declarations
typedef struct latency_tracker latency_tracker_t;
typedef struct circuit_breaker circuit_breaker_t;
typedef struct retry_policy retry_policy_t;
typedef struct timeout_manager timeout_manager_t;
typedef struct bottleneck_analyzer bottleneck_analyzer_t;

// =============================================================================
// LATENCY TRACKING & PERCENTILES
// =============================================================================

typedef struct {
    uint64_t sample_count;
    double min_ms;
    double max_ms;
    double mean_ms;
    double median_ms;
    double stddev_ms;
    double p50_ms;
    double p75_ms;
    double p90_ms;
    double p95_ms;
    double p99_ms;
    double p999_ms;
} latency_stats_t;

typedef struct {
    char operation[128];
    latency_stats_t stats;
    uint64_t error_count;
    double error_rate;
    uint64_t timeout_count;
} operation_latency_t;

typedef struct {
    size_t window_size;
    uint64_t window_duration_ms;
    size_t bucket_count;
    bool track_per_operation;
} latency_config_t;

// Latency tracker management
latency_tracker_t* latency_tracker_create(latency_config_t* config);
void latency_tracker_destroy(latency_tracker_t* tracker);

// Recording latencies
int latency_record(latency_tracker_t* tracker, const char* operation,
                  double latency_ms, bool success);
int latency_record_start(latency_tracker_t* tracker, const char* operation,
                         uint64_t* request_id);
int latency_record_end(latency_tracker_t* tracker, uint64_t request_id,
                      bool success);

// Retrieving statistics
int latency_get_stats(latency_tracker_t* tracker, const char* operation,
                     latency_stats_t* stats);
int latency_get_all_operations(latency_tracker_t* tracker,
                               operation_latency_t** operations,
                               size_t* count);
int latency_reset_stats(latency_tracker_t* tracker);

// =============================================================================
// TAIL LATENCY ANALYSIS
// =============================================================================

typedef enum {
    TAIL_LATENCY_NETWORK,        // Network delays
    TAIL_LATENCY_GC,             // Garbage collection
    TAIL_LATENCY_DISK_IO,        // Disk I/O
    TAIL_LATENCY_LOCK_CONTENTION,// Lock contention
    TAIL_LATENCY_QUEUEING,       // Queueing delays
    TAIL_LATENCY_CACHE_MISS,     // Cache misses
    TAIL_LATENCY_RESOURCE_EXHAUSTION, // CPU/Memory exhaustion
    TAIL_LATENCY_EXTERNAL_DEPENDENCY  // External service
} tail_latency_cause_t;

typedef struct {
    char operation[128];
    uint64_t timestamp;
    double latency_ms;
    tail_latency_cause_t likely_cause;
    char details[512];
    double severity;             // 0.0-1.0
} tail_latency_event_t;

typedef struct {
    double threshold_percentile; // e.g., 0.99 for P99
    double threshold_multiplier; // Alert if > multiplier * median
    bool enable_auto_detection;
    bool enable_cause_analysis;
} tail_latency_config_t;

// Tail latency detection
int latency_detect_tail_events(latency_tracker_t* tracker,
                               tail_latency_config_t* config,
                               tail_latency_event_t** events,
                               size_t* count);

// Tail latency mitigation strategies
typedef enum {
    TAIL_MITIGATION_HEDGING,         // Send redundant requests
    TAIL_MITIGATION_SPECULATION,     // Speculative execution
    TAIL_MITIGATION_LOAD_SHEDDING,   // Drop low-priority requests
    TAIL_MITIGATION_CACHING,         // Aggressive caching
    TAIL_MITIGATION_CIRCUIT_BREAKER  // Circuit breaker pattern
} tail_latency_mitigation_t;

int latency_suggest_mitigations(tail_latency_event_t* event,
                                tail_latency_mitigation_t** strategies,
                                size_t* count);

// =============================================================================
// LATENCY BUDGETING
// =============================================================================

typedef struct {
    char operation[128];
    double allocated_budget_ms;
    double consumed_ms;
    double remaining_ms;
    double utilization;          // 0.0-1.0
    bool is_exceeded;
} latency_budget_t;

typedef struct {
    char service_name[128];
    latency_budget_t* budgets;
    size_t budget_count;
    double total_budget_ms;
    double total_consumed_ms;
} service_latency_budget_t;

// Budget management
service_latency_budget_t* latency_budget_create(const char* service_name);
void latency_budget_destroy(service_latency_budget_t* budget);

int latency_budget_set(service_latency_budget_t* budget, const char* operation,
                       double budget_ms);
int latency_budget_consume(service_latency_budget_t* budget, 
                          const char* operation, double consumed_ms);
int latency_budget_get(service_latency_budget_t* budget, const char* operation,
                      latency_budget_t* info);
int latency_budget_check_exceeded(service_latency_budget_t* budget,
                                  char*** exceeded_operations, size_t* count);

// =============================================================================
// TIMEOUT MANAGEMENT
// =============================================================================

typedef enum {
    TIMEOUT_TYPE_CONNECT,        // Connection timeout
    TIMEOUT_TYPE_READ,           // Read timeout
    TIMEOUT_TYPE_WRITE,          // Write timeout
    TIMEOUT_TYPE_REQUEST,        // Overall request timeout
    TIMEOUT_TYPE_IDLE            // Idle connection timeout
} timeout_type_t;

typedef struct {
    timeout_type_t type;
    uint64_t duration_ms;
    bool is_adaptive;            // Adjust based on latency
    double percentile_target;    // e.g., 0.95 for P95
} timeout_config_t;

typedef struct {
    char operation[128];
    timeout_type_t type;
    uint64_t timeout_ms;
    uint64_t actual_duration_ms;
    uint64_t timestamp;
    char error_message[256];
} timeout_event_t;

// Timeout manager
timeout_manager_t* timeout_manager_create(void);
void timeout_manager_destroy(timeout_manager_t* manager);

int timeout_set(timeout_manager_t* manager, const char* operation,
               timeout_type_t type, uint64_t timeout_ms);
int timeout_get(timeout_manager_t* manager, const char* operation,
               timeout_type_t type, uint64_t* timeout_ms);

// Adaptive timeouts (based on latency history)
int timeout_calculate_adaptive(timeout_manager_t* manager,
                               latency_tracker_t* tracker,
                               const char* operation,
                               timeout_type_t type,
                               uint64_t* timeout_ms);
int timeout_update_adaptive(timeout_manager_t* manager,
                           latency_tracker_t* tracker);

// Timeout tracking
int timeout_record_event(timeout_manager_t* manager, timeout_event_t* event);
int timeout_get_events(timeout_manager_t* manager, timeout_event_t** events,
                      size_t* count);

// =============================================================================
// RETRY POLICIES
// =============================================================================

typedef enum {
    RETRY_STRATEGY_FIXED,        // Fixed delay between retries
    RETRY_STRATEGY_LINEAR,       // Linearly increasing delay
    RETRY_STRATEGY_EXPONENTIAL,  // Exponential backoff
    RETRY_STRATEGY_DECORRELATED  // AWS-style decorrelated jitter
} retry_strategy_t;

typedef struct {
    retry_strategy_t strategy;
    size_t max_attempts;
    uint64_t initial_delay_ms;
    uint64_t max_delay_ms;
    double backoff_multiplier;   // For exponential backoff
    double jitter_factor;        // 0.0-1.0, add randomness
    bool retry_on_timeout;
    int* retryable_status_codes;
    size_t status_code_count;
} retry_config_t;

typedef struct {
    size_t attempt_number;
    uint64_t next_retry_delay_ms;
    bool should_retry;
    char reason[256];
} retry_decision_t;

typedef struct {
    char operation[128];
    size_t total_attempts;
    size_t successful_attempts;
    size_t failed_attempts;
    uint64_t total_delay_ms;
    double success_rate;
} retry_stats_t;

// Retry policy management
retry_policy_t* retry_policy_create(retry_config_t* config);
void retry_policy_destroy(retry_policy_t* policy);

// Retry decision
retry_decision_t* retry_should_retry(retry_policy_t* policy, size_t attempt,
                                     int error_code, uint64_t latency_ms);
uint64_t retry_calculate_delay(retry_policy_t* policy, size_t attempt);

// Retry execution
typedef int (*retryable_fn)(void* ctx, void** result);

int retry_execute(retry_policy_t* policy, retryable_fn function, void* ctx,
                 void** result);
int retry_execute_async(retry_policy_t* policy, retryable_fn function,
                       void* ctx, void (*callback)(void* result, void* ctx));

// Retry statistics
int retry_get_stats(retry_policy_t* policy, const char* operation,
                   retry_stats_t* stats);

// =============================================================================
// CIRCUIT BREAKER
// =============================================================================

typedef enum {
    CIRCUIT_STATE_CLOSED,        // Normal operation
    CIRCUIT_STATE_OPEN,          // Failing, reject requests
    CIRCUIT_STATE_HALF_OPEN      // Testing if service recovered
} circuit_state_t;

typedef struct {
    size_t failure_threshold;    // Failures before opening
    uint64_t timeout_ms;         // Time to wait before half-open
    size_t success_threshold;    // Successes to close from half-open
    uint64_t monitoring_window_ms;
    double error_rate_threshold; // 0.0-1.0
} circuit_breaker_config_t;

typedef struct {
    circuit_state_t state;
    uint64_t state_changed_at;
    uint64_t total_requests;
    uint64_t successful_requests;
    uint64_t failed_requests;
    uint64_t rejected_requests;
    double error_rate;
    uint64_t time_in_open_ms;
} circuit_breaker_stats_t;

// Circuit breaker management
circuit_breaker_t* circuit_breaker_create(const char* name,
                                          circuit_breaker_config_t* config);
void circuit_breaker_destroy(circuit_breaker_t* breaker);

// Circuit breaker operations
bool circuit_breaker_allow_request(circuit_breaker_t* breaker);
int circuit_breaker_record_success(circuit_breaker_t* breaker);
int circuit_breaker_record_failure(circuit_breaker_t* breaker);
circuit_state_t circuit_breaker_get_state(circuit_breaker_t* breaker);

// Circuit breaker execution
typedef int (*circuit_fn)(void* ctx, void** result);
typedef int (*fallback_fn)(void* ctx, void** result);

int circuit_breaker_execute(circuit_breaker_t* breaker, circuit_fn function,
                           fallback_fn fallback, void* ctx, void** result);

// Circuit breaker statistics
int circuit_breaker_get_stats(circuit_breaker_t* breaker,
                              circuit_breaker_stats_t* stats);
int circuit_breaker_reset(circuit_breaker_t* breaker);
int circuit_breaker_force_open(circuit_breaker_t* breaker);
int circuit_breaker_force_close(circuit_breaker_t* breaker);

// =============================================================================
// DISTRIBUTED TRACING
// =============================================================================

typedef struct {
    char trace_id[64];
    char span_id[64];
    char parent_span_id[64];
    char operation_name[128];
    char service_name[128];
    uint64_t start_time_us;
    uint64_t duration_us;
    char** tags;
    size_t tag_count;
    char** logs;
    size_t log_count;
} trace_span_t;

typedef struct {
    char trace_id[64];
    trace_span_t** spans;
    size_t span_count;
    uint64_t total_duration_us;
    char root_operation[128];
} trace_t;

typedef struct {
    bool enable_sampling;
    double sampling_rate;        // 0.0-1.0
    size_t max_spans_per_trace;
    bool enable_baggage;
} tracing_config_t;

// Span management
trace_span_t* trace_span_start(const char* operation_name,
                               const char* service_name,
                               const char* parent_span_id);
void trace_span_finish(trace_span_t* span);
int trace_span_add_tag(trace_span_t* span, const char* key, const char* value);
int trace_span_add_log(trace_span_t* span, const char* message);
void trace_span_destroy(trace_span_t* span);

// Trace context propagation
int trace_inject_context(trace_span_t* span, char** headers, size_t* count);
trace_span_t* trace_extract_context(char** headers, size_t count,
                                    const char* operation_name,
                                    const char* service_name);

// Trace collection
trace_t* trace_get(const char* trace_id);
void trace_destroy(trace_t* trace);

// =============================================================================
// BOTTLENECK ANALYSIS
// =============================================================================

typedef enum {
    BOTTLENECK_CPU,
    BOTTLENECK_MEMORY,
    BOTTLENECK_DISK_IO,
    BOTTLENECK_NETWORK_IO,
    BOTTLENECK_DATABASE,
    BOTTLENECK_EXTERNAL_API,
    BOTTLENECK_LOCK_CONTENTION,
    BOTTLENECK_CACHE_MISS
} bottleneck_type_t;

typedef struct {
    char operation[128];
    bottleneck_type_t type;
    double time_spent_ms;
    double percentage_of_total;
    char description[512];
    char** recommendations;
    size_t recommendation_count;
} bottleneck_t;

typedef struct {
    char service_name[128];
    uint64_t analysis_duration_ms;
    bottleneck_t* bottlenecks;
    size_t bottleneck_count;
    double total_time_analyzed_ms;
} bottleneck_report_t;

// Bottleneck analyzer
bottleneck_analyzer_t* bottleneck_analyzer_create(const char* service_name);
void bottleneck_analyzer_destroy(bottleneck_analyzer_t* analyzer);

// Record operation breakdown
int bottleneck_record_operation(bottleneck_analyzer_t* analyzer,
                                const char* operation,
                                bottleneck_type_t type,
                                double duration_ms);

// Analysis
bottleneck_report_t* bottleneck_analyze(bottleneck_analyzer_t* analyzer,
                                        uint64_t time_window_ms);
void bottleneck_report_destroy(bottleneck_report_t* report);

// Critical path analysis
typedef struct {
    char** operations;
    size_t operation_count;
    double total_duration_ms;
    bool is_serial;
} critical_path_t;

critical_path_t* bottleneck_find_critical_path(trace_t* trace);
void critical_path_destroy(critical_path_t* path);

// =============================================================================
// PERFORMANCE PROFILING
// =============================================================================

typedef struct {
    char function_name[256];
    uint64_t call_count;
    double total_time_ms;
    double self_time_ms;
    double avg_time_ms;
    double min_time_ms;
    double max_time_ms;
} profile_entry_t;

typedef struct {
    char service_name[128];
    uint64_t profiling_duration_ms;
    profile_entry_t* entries;
    size_t entry_count;
    double total_cpu_time_ms;
} profile_report_t;

// Profiling
int profile_start(const char* service_name);
int profile_stop(const char* service_name);
profile_report_t* profile_get_report(const char* service_name);
void profile_report_destroy(profile_report_t* report);

// Function profiling
typedef struct profiler profiler_t;

profiler_t* profiler_create(void);
void profiler_destroy(profiler_t* profiler);
int profiler_enter(profiler_t* profiler, const char* function_name);
int profiler_exit(profiler_t* profiler, const char* function_name);
profile_report_t* profiler_get_report(profiler_t* profiler);

// =============================================================================
// OBSERVABILITY DASHBOARD
// =============================================================================

typedef struct {
    latency_stats_t latency;
    circuit_breaker_stats_t circuit_breaker;
    retry_stats_t retry;
    uint64_t total_requests;
    uint64_t successful_requests;
    uint64_t failed_requests;
    double error_rate;
    double requests_per_second;
} observability_metrics_t;

// Metrics collection
observability_metrics_t* observability_collect_metrics(const char* service_name);
void observability_metrics_destroy(observability_metrics_t* metrics);

// Metrics export
int observability_export_prometheus(observability_metrics_t* metrics,
                                    char* buffer, size_t size);
int observability_export_json(observability_metrics_t* metrics,
                              char* buffer, size_t size);

// Health checks
typedef enum {
    HEALTH_STATUS_HEALTHY,
    HEALTH_STATUS_DEGRADED,
    HEALTH_STATUS_UNHEALTHY
} health_status_t;

typedef struct {
    health_status_t status;
    char message[256];
    double uptime_seconds;
    double cpu_usage;
    double memory_usage_mb;
} health_check_t;

health_check_t* observability_health_check(const char* service_name);

#endif // LATENCY_OBSERVABILITY_H
