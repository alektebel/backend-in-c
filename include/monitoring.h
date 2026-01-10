#ifndef MONITORING_H
#define MONITORING_H

#include "common.h"

// ============================================================================
// Metrics Collection (Prometheus-style)
// ============================================================================

typedef enum {
    METRIC_TYPE_COUNTER,
    METRIC_TYPE_GAUGE,
    METRIC_TYPE_HISTOGRAM,
    METRIC_TYPE_SUMMARY
} metric_type_t;

typedef struct metrics_registry metrics_registry_t;
typedef struct metric metric_t;
typedef struct counter counter_t;
typedef struct gauge gauge_t;
typedef struct histogram histogram_t;
typedef struct summary summary_t;

// Metrics registry
metrics_registry_t* metrics_registry_create(void);
void metrics_registry_destroy(metrics_registry_t* registry);
char* metrics_registry_export_prometheus(metrics_registry_t* registry);
char* metrics_registry_export_json(metrics_registry_t* registry);

// Counter metric (monotonically increasing)
counter_t* counter_create(const char* name, const char* help);
void counter_destroy(counter_t* counter);
int counter_inc(counter_t* counter);
int counter_add(counter_t* counter, double value);
double counter_get(const counter_t* counter);
int metrics_registry_register_counter(metrics_registry_t* registry, counter_t* counter);

// Gauge metric (can go up and down)
gauge_t* gauge_create(const char* name, const char* help);
void gauge_destroy(gauge_t* gauge);
int gauge_set(gauge_t* gauge, double value);
int gauge_inc(gauge_t* gauge);
int gauge_dec(gauge_t* gauge);
int gauge_add(gauge_t* gauge, double value);
int gauge_sub(gauge_t* gauge, double value);
double gauge_get(const gauge_t* gauge);
int metrics_registry_register_gauge(metrics_registry_t* registry, gauge_t* gauge);

// Histogram metric (for measuring distributions)
histogram_t* histogram_create(const char* name, const char* help,
                             const double* buckets, size_t bucket_count);
void histogram_destroy(histogram_t* histogram);
int histogram_observe(histogram_t* histogram, double value);
double histogram_get_sum(const histogram_t* histogram);
uint64_t histogram_get_count(const histogram_t* histogram);
int metrics_registry_register_histogram(metrics_registry_t* registry,
                                       histogram_t* histogram);

// Summary metric (for percentiles)
summary_t* summary_create(const char* name, const char* help);
void summary_destroy(summary_t* summary);
int summary_observe(summary_t* summary, double value);
double summary_get_quantile(summary_t* summary, double quantile);
int metrics_registry_register_summary(metrics_registry_t* registry, summary_t* summary);

// Labels for metrics
typedef struct metric_labels metric_labels_t;

metric_labels_t* metric_labels_create(void);
void metric_labels_destroy(metric_labels_t* labels);
int metric_labels_add(metric_labels_t* labels, const char* key, const char* value);
int counter_with_labels(counter_t* counter, metric_labels_t* labels, double value);
int gauge_with_labels(gauge_t* gauge, metric_labels_t* labels, double value);

// ============================================================================
// Health Checks
// ============================================================================

typedef enum {
    HEALTH_STATUS_UP,
    HEALTH_STATUS_DOWN,
    HEALTH_STATUS_DEGRADED
} health_status_t;

typedef struct health_check health_check_t;
typedef struct health_check_result health_check_result_t;

struct health_check_result {
    health_status_t status;
    char* message;
    uint64_t response_time_ms;
    char** details;
    size_t detail_count;
};

// Health check operations
health_check_t* health_check_create(const char* name);
void health_check_destroy(health_check_t* check);
int health_check_add_check(health_check_t* check,
                          health_check_result_t* (*check_func)(void*),
                          void* user_data);
health_check_result_t* health_check_execute(health_check_t* check);
void health_check_result_destroy(health_check_result_t* result);
char* health_check_result_to_json(const health_check_result_t* result);

// ============================================================================
// Alerting
// ============================================================================

typedef enum {
    ALERT_SEVERITY_INFO,
    ALERT_SEVERITY_WARNING,
    ALERT_SEVERITY_CRITICAL
} alert_severity_t;

typedef enum {
    ALERT_STATE_PENDING,
    ALERT_STATE_FIRING,
    ALERT_STATE_RESOLVED
} alert_state_t;

typedef struct alert alert_t;
typedef struct alert_manager alert_manager_t;
typedef struct alert_rule alert_rule_t;

// Alert operations
alert_t* alert_create(const char* name, alert_severity_t severity, 
                     const char* message);
void alert_destroy(alert_t* alert);
int alert_add_label(alert_t* alert, const char* key, const char* value);
int alert_add_annotation(alert_t* alert, const char* key, const char* value);
alert_state_t alert_get_state(const alert_t* alert);

// Alert manager
alert_manager_t* alert_manager_create(void);
void alert_manager_destroy(alert_manager_t* manager);
int alert_manager_add_rule(alert_manager_t* manager, alert_rule_t* rule);
int alert_manager_evaluate_rules(alert_manager_t* manager);
int alert_manager_send_alert(alert_manager_t* manager, alert_t* alert);
int alert_manager_resolve_alert(alert_manager_t* manager, const char* alert_name);

// Alert rules
alert_rule_t* alert_rule_create(const char* name, const char* expression,
                               alert_severity_t severity, uint64_t duration_ms);
void alert_rule_destroy(alert_rule_t* rule);
int alert_rule_evaluate(alert_rule_t* rule, metrics_registry_t* registry);

// ============================================================================
// Alert Channels
// ============================================================================

typedef struct alert_channel alert_channel_t;

typedef enum {
    ALERT_CHANNEL_EMAIL,
    ALERT_CHANNEL_SLACK,
    ALERT_CHANNEL_WEBHOOK,
    ALERT_CHANNEL_PAGERDUTY,
    ALERT_CHANNEL_SMS
} alert_channel_type_t;

alert_channel_t* alert_channel_create(alert_channel_type_t type, 
                                     const char* config);
void alert_channel_destroy(alert_channel_t* channel);
int alert_channel_send(alert_channel_t* channel, const alert_t* alert);
int alert_manager_add_channel(alert_manager_t* manager, alert_channel_t* channel);

// ============================================================================
// Dashboards and Visualization
// ============================================================================

typedef struct dashboard dashboard_t;
typedef struct panel panel_t;

dashboard_t* dashboard_create(const char* title);
void dashboard_destroy(dashboard_t* dashboard);
int dashboard_add_panel(dashboard_t* dashboard, panel_t* panel);
char* dashboard_export_json(const dashboard_t* dashboard);

// Panel types
panel_t* panel_create_graph(const char* title, const char* query);
panel_t* panel_create_gauge(const char* title, const char* query, 
                           double min, double max);
panel_t* panel_create_table(const char* title, const char* query);
void panel_destroy(panel_t* panel);

// ============================================================================
// Application Performance Monitoring (APM)
// ============================================================================

typedef struct apm_agent apm_agent_t;
typedef struct apm_transaction apm_transaction_t;
typedef struct apm_span apm_span_t;

// APM agent
apm_agent_t* apm_agent_create(const char* service_name, const char* server_url);
void apm_agent_destroy(apm_agent_t* agent);
int apm_agent_start(apm_agent_t* agent);
int apm_agent_stop(apm_agent_t* agent);

// Transaction tracking
apm_transaction_t* apm_transaction_start(apm_agent_t* agent, const char* name,
                                        const char* type);
void apm_transaction_end(apm_transaction_t* transaction);
void apm_transaction_set_result(apm_transaction_t* transaction, const char* result);
void apm_transaction_set_user(apm_transaction_t* transaction, const char* user_id);
void apm_transaction_add_label(apm_transaction_t* transaction, 
                              const char* key, const char* value);

// Span tracking (sub-operations within a transaction)
apm_span_t* apm_span_start(apm_transaction_t* transaction, const char* name,
                          const char* type);
void apm_span_end(apm_span_t* span);
void apm_span_set_stacktrace(apm_span_t* span);

// ============================================================================
// System Metrics Collection
// ============================================================================

typedef struct system_metrics system_metrics_t;

struct system_metrics {
    double cpu_usage_percent;
    uint64_t memory_used_bytes;
    uint64_t memory_available_bytes;
    double disk_usage_percent;
    uint64_t disk_read_bytes_per_sec;
    uint64_t disk_write_bytes_per_sec;
    uint64_t network_in_bytes_per_sec;
    uint64_t network_out_bytes_per_sec;
    int open_file_descriptors;
    int thread_count;
    double load_average_1min;
    double load_average_5min;
    double load_average_15min;
};

int collect_system_metrics(system_metrics_t* metrics);
int register_system_metrics(metrics_registry_t* registry);

// ============================================================================
// HTTP Metrics
// ============================================================================

typedef struct http_metrics http_metrics_t;

http_metrics_t* http_metrics_create(void);
void http_metrics_destroy(http_metrics_t* metrics);
int http_metrics_record_request(http_metrics_t* metrics, const char* method,
                                const char* path, int status_code,
                                uint64_t duration_ms);
int http_metrics_register(http_metrics_t* metrics, metrics_registry_t* registry);

// ============================================================================
// Database Metrics
// ============================================================================

typedef struct db_metrics db_metrics_t;

db_metrics_t* db_metrics_create(void);
void db_metrics_destroy(db_metrics_t* metrics);
int db_metrics_record_query(db_metrics_t* metrics, const char* query_type,
                           uint64_t duration_ms, int success);
int db_metrics_set_connection_pool_stats(db_metrics_t* metrics,
                                        int active_connections,
                                        int idle_connections,
                                        int waiting_connections);
int db_metrics_register(db_metrics_t* metrics, metrics_registry_t* registry);

#endif // MONITORING_H
