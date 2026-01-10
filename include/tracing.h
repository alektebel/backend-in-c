#ifndef TRACING_H
#define TRACING_H

#include "common.h"

// ============================================================================
// Distributed Tracing (OpenTelemetry/Jaeger-style)
// ============================================================================

typedef struct trace_context trace_context_t;
typedef struct span span_t;
typedef struct tracer tracer_t;
typedef struct trace_exporter trace_exporter_t;

// Trace context (W3C Trace Context standard)
struct trace_context {
    char trace_id[33];      // 128-bit trace ID as hex string
    char span_id[17];       // 64-bit span ID as hex string
    char parent_span_id[17]; // 64-bit parent span ID as hex string
    int sampled;
    char* trace_state;
};

// Span kind
typedef enum {
    SPAN_KIND_INTERNAL,
    SPAN_KIND_SERVER,
    SPAN_KIND_CLIENT,
    SPAN_KIND_PRODUCER,
    SPAN_KIND_CONSUMER
} span_kind_t;

// Span status
typedef enum {
    SPAN_STATUS_UNSET,
    SPAN_STATUS_OK,
    SPAN_STATUS_ERROR
} span_status_t;

// ============================================================================
// Tracer Operations
// ============================================================================

tracer_t* tracer_create(const char* service_name, const char* service_version);
void tracer_destroy(tracer_t* tracer);
int tracer_set_exporter(tracer_t* tracer, trace_exporter_t* exporter);
int tracer_set_sampler(tracer_t* tracer, double sampling_rate);

// ============================================================================
// Span Operations
// ============================================================================

span_t* span_create(tracer_t* tracer, const char* operation_name, 
                   span_kind_t kind);
span_t* span_create_child(span_t* parent, const char* operation_name,
                          span_kind_t kind);
void span_destroy(span_t* span);
void span_finish(span_t* span);

// Span attributes
int span_set_attribute_string(span_t* span, const char* key, const char* value);
int span_set_attribute_int(span_t* span, const char* key, int64_t value);
int span_set_attribute_double(span_t* span, const char* key, double value);
int span_set_attribute_bool(span_t* span, const char* key, int value);

// Span status and errors
int span_set_status(span_t* span, span_status_t status, const char* description);
int span_record_exception(span_t* span, const char* exception_type,
                         const char* message, const char* stacktrace);

// Span events (logs within span)
int span_add_event(span_t* span, const char* name);
int span_add_event_with_attributes(span_t* span, const char* name,
                                  const char** keys, const char** values,
                                  size_t count);

// Context propagation
trace_context_t* span_get_context(const span_t* span);
span_t* span_from_context(tracer_t* tracer, const trace_context_t* context,
                         const char* operation_name, span_kind_t kind);

// ============================================================================
// Trace Context Propagation
// ============================================================================

// Inject trace context into headers (for HTTP propagation)
int trace_context_inject_http(const trace_context_t* context, 
                              char** headers, size_t* header_count);
trace_context_t* trace_context_extract_http(const char** headers, size_t header_count);

// Inject trace context into metadata (for gRPC/message queues)
char* trace_context_inject_binary(const trace_context_t* context);
trace_context_t* trace_context_extract_binary(const char* binary_context);

void trace_context_destroy(trace_context_t* context);

// ============================================================================
// Trace Exporters
// ============================================================================

typedef enum {
    TRACE_EXPORTER_JAEGER,
    TRACE_EXPORTER_ZIPKIN,
    TRACE_EXPORTER_OTLP,  // OpenTelemetry Protocol
    TRACE_EXPORTER_CONSOLE
} trace_exporter_type_t;

// Jaeger exporter
trace_exporter_t* trace_exporter_jaeger_create(const char* agent_host, int agent_port);

// Zipkin exporter
trace_exporter_t* trace_exporter_zipkin_create(const char* collector_url);

// OTLP exporter
trace_exporter_t* trace_exporter_otlp_create(const char* endpoint);

// Console exporter (for debugging)
trace_exporter_t* trace_exporter_console_create(void);

void trace_exporter_destroy(trace_exporter_t* exporter);
int trace_exporter_export(trace_exporter_t* exporter, span_t** spans, size_t count);

// ============================================================================
// Sampling Strategies
// ============================================================================

typedef struct trace_sampler trace_sampler_t;

typedef enum {
    SAMPLER_ALWAYS_ON,
    SAMPLER_ALWAYS_OFF,
    SAMPLER_PROBABILITY,    // Sample based on probability
    SAMPLER_RATE_LIMITING,  // Sample based on rate limit
    SAMPLER_PARENT_BASED   // Sample based on parent span decision
} sampler_type_t;

trace_sampler_t* trace_sampler_create(sampler_type_t type, double parameter);
void trace_sampler_destroy(trace_sampler_t* sampler);
int trace_sampler_should_sample(trace_sampler_t* sampler, 
                               const trace_context_t* context);

// ============================================================================
// Baggage (Cross-cutting concerns)
// ============================================================================

typedef struct baggage baggage_t;

baggage_t* baggage_create(void);
void baggage_destroy(baggage_t* baggage);
int baggage_set(baggage_t* baggage, const char* key, const char* value);
const char* baggage_get(const baggage_t* baggage, const char* key);
int baggage_remove(baggage_t* baggage, const char* key);
char* baggage_serialize(const baggage_t* baggage);
baggage_t* baggage_deserialize(const char* serialized);

// Associate baggage with span
int span_set_baggage(span_t* span, baggage_t* baggage);
baggage_t* span_get_baggage(const span_t* span);

// ============================================================================
// Span Links
// ============================================================================

typedef struct span_link span_link_t;

span_link_t* span_link_create(const trace_context_t* linked_context);
void span_link_destroy(span_link_t* link);
int span_link_add_attribute(span_link_t* link, const char* key, const char* value);
int span_add_link(span_t* span, span_link_t* link);

// ============================================================================
// Trace Analysis and Aggregation
// ============================================================================

typedef struct trace_analyzer trace_analyzer_t;
typedef struct trace_stats trace_stats_t;

struct trace_stats {
    uint64_t total_traces;
    uint64_t total_spans;
    uint64_t error_count;
    double avg_duration_ms;
    double p50_duration_ms;
    double p95_duration_ms;
    double p99_duration_ms;
    char** slow_operations;
    size_t slow_operations_count;
};

trace_analyzer_t* trace_analyzer_create(void);
void trace_analyzer_destroy(trace_analyzer_t* analyzer);
int trace_analyzer_add_span(trace_analyzer_t* analyzer, const span_t* span);
trace_stats_t* trace_analyzer_get_stats(trace_analyzer_t* analyzer, 
                                       const char* service_name,
                                       uint64_t start_time, uint64_t end_time);
void trace_stats_destroy(trace_stats_t* stats);

// ============================================================================
// Service Dependency Graph
// ============================================================================

typedef struct dependency_graph dependency_graph_t;
typedef struct service_dependency service_dependency_t;

struct service_dependency {
    char* source_service;
    char* target_service;
    uint64_t call_count;
    double avg_duration_ms;
    double error_rate;
};

dependency_graph_t* dependency_graph_create(void);
void dependency_graph_destroy(dependency_graph_t* graph);
int dependency_graph_add_span(dependency_graph_t* graph, const span_t* span);
service_dependency_t** dependency_graph_get_dependencies(dependency_graph_t* graph,
                                                        size_t* count);
char* dependency_graph_export_dot(const dependency_graph_t* graph);
void service_dependency_destroy(service_dependency_t* dep);

// ============================================================================
// Trace Correlation with Logs
// ============================================================================

typedef struct trace_log_correlator trace_log_correlator_t;

trace_log_correlator_t* trace_log_correlator_create(void);
void trace_log_correlator_destroy(trace_log_correlator_t* correlator);
int trace_log_correlator_add_trace_id_to_log(trace_log_correlator_t* correlator,
                                            const char* trace_id,
                                            const char* log_entry);
char** trace_log_correlator_get_logs_for_trace(trace_log_correlator_t* correlator,
                                               const char* trace_id,
                                               size_t* count);

#endif // TRACING_H
