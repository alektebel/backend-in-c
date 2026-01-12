#ifndef NETWORK_SERIALIZATION_H
#define NETWORK_SERIALIZATION_H

#include "common.h"
#include <stdint.h>
#include <stdbool.h>

// =============================================================================
// NETWORK & SERIALIZATION COSTS
// =============================================================================
// This module provides templates for:
// - Payload size optimization
// - Compression algorithms and strategies
// - JSON vs Protobuf vs other serialization formats
// - gRPC vs REST performance comparison
// - Connection reuse and keep-alive management
// - Network protocol optimization

// Forward declarations
typedef struct serializer serializer_t;
typedef struct compressor compressor_t;
typedef struct connection_manager connection_manager_t;
typedef struct protocol_handler protocol_handler_t;

// =============================================================================
// SERIALIZATION FORMATS
// =============================================================================

typedef enum {
    SERIALIZATION_JSON,          // Human-readable, larger payload
    SERIALIZATION_PROTOBUF,      // Binary, compact, schema-based
    SERIALIZATION_MSGPACK,       // Binary, compact, schemaless
    SERIALIZATION_AVRO,          // Binary, compact, schema-based
    SERIALIZATION_THRIFT,        // Binary, compact, schema-based
    SERIALIZATION_CBOR,          // Binary JSON (RFC 8949)
    SERIALIZATION_XML,           // Human-readable, verbose
    SERIALIZATION_BSON           // Binary JSON (MongoDB)
} serialization_format_t;

typedef struct {
    serialization_format_t format;
    bool pretty_print;           // For human-readable formats
    bool validate_schema;
    bool enable_compression;
    size_t buffer_size;
} serialization_config_t;

typedef struct {
    size_t serialized_size;
    size_t original_size;
    double compression_ratio;
    double serialization_time_ms;
    double deserialization_time_ms;
} serialization_stats_t;

// Serializer management
serializer_t* serializer_create(serialization_config_t* config);
void serializer_destroy(serializer_t* serializer);

// Serialization operations
int serialize_to_buffer(serializer_t* serializer, const void* object,
                       void** buffer, size_t* size);
int deserialize_from_buffer(serializer_t* serializer, const void* buffer,
                            size_t size, void** object);
int serialize_to_file(serializer_t* serializer, const void* object,
                     const char* filename);
int deserialize_from_file(serializer_t* serializer, const char* filename,
                          void** object);

// Format conversion
int convert_format(const void* input, size_t input_size,
                  serialization_format_t from_format,
                  serialization_format_t to_format,
                  void** output, size_t* output_size);

// Serialization statistics
int serializer_get_stats(serializer_t* serializer, serialization_stats_t* stats);

// =============================================================================
// PAYLOAD SIZE OPTIMIZATION
// =============================================================================

typedef struct {
    char field_name[128];
    size_t size_bytes;
    double percentage;
    bool is_optional;
    bool is_redundant;
} field_analysis_t;

typedef struct {
    size_t original_size;
    size_t optimized_size;
    size_t bytes_saved;
    double reduction_percentage;
    field_analysis_t* field_analyses;
    size_t field_count;
    char** optimization_suggestions;
    size_t suggestion_count;
} payload_analysis_t;

// Payload analysis
payload_analysis_t* analyze_payload_size(const void* payload, size_t size,
                                         serialization_format_t format);
void payload_analysis_destroy(payload_analysis_t* analysis);

// Payload optimization strategies
typedef enum {
    OPTIMIZE_REMOVE_NULL,        // Remove null/empty fields
    OPTIMIZE_FIELD_ABBREVIATION, // Shorten field names
    OPTIMIZE_DELTA_ENCODING,     // Send only changes
    OPTIMIZE_REFERENCE_IDS,      // Use IDs instead of nested objects
    OPTIMIZE_FIELD_SELECTION     // Send only requested fields
} payload_optimization_t;

int optimize_payload(const void* payload, size_t size,
                    payload_optimization_t* strategies, size_t strategy_count,
                    void** optimized, size_t* optimized_size);

// Field filtering (GraphQL-style)
typedef struct {
    char** fields;
    size_t field_count;
    bool include_nested;
} field_selector_t;

int filter_payload_fields(const void* payload, size_t size,
                         field_selector_t* selector,
                         void** filtered, size_t* filtered_size);

// =============================================================================
// COMPRESSION STRATEGIES
// =============================================================================

typedef enum {
    COMPRESSION_NONE,
    COMPRESSION_GZIP,            // Good balance, widely supported
    COMPRESSION_DEFLATE,         // Similar to gzip
    COMPRESSION_BROTLI,          // Better compression, slower
    COMPRESSION_ZSTD,            // Fast, good ratio (Facebook)
    COMPRESSION_LZ4,             // Very fast, lower ratio
    COMPRESSION_SNAPPY,          // Very fast, lower ratio (Google)
    COMPRESSION_LZO              // Fast decompression
} compression_algorithm_t;

typedef enum {
    COMPRESSION_LEVEL_FASTEST = 1,
    COMPRESSION_LEVEL_FAST = 3,
    COMPRESSION_LEVEL_DEFAULT = 6,
    COMPRESSION_LEVEL_BEST = 9
} compression_level_t;

typedef struct {
    compression_algorithm_t algorithm;
    compression_level_t level;
    size_t min_size_to_compress; // Don't compress if smaller
    bool streaming;              // Stream compression
} compression_config_t;

typedef struct {
    size_t original_size;
    size_t compressed_size;
    double compression_ratio;
    double compression_time_ms;
    double decompression_time_ms;
    double throughput_mbps;
} compression_stats_t;

// Compressor management
compressor_t* compressor_create(compression_config_t* config);
void compressor_destroy(compressor_t* compressor);

// Compression operations
int compress_data(compressor_t* compressor, const void* input, size_t input_size,
                 void** output, size_t* output_size);
int decompress_data(compressor_t* compressor, const void* input, size_t input_size,
                   void** output, size_t* output_size);

// Streaming compression
typedef struct compression_stream compression_stream_t;

compression_stream_t* compression_stream_create(compressor_t* compressor);
void compression_stream_destroy(compression_stream_t* stream);
int compression_stream_write(compression_stream_t* stream, const void* data,
                             size_t size);
int compression_stream_read(compression_stream_t* stream, void* buffer,
                            size_t size, size_t* bytes_read);
int compression_stream_finish(compression_stream_t* stream);

// Compression statistics
int compressor_get_stats(compressor_t* compressor, compression_stats_t* stats);

// Compression benchmarking
typedef struct {
    compression_algorithm_t algorithm;
    compression_level_t level;
    double compression_time_ms;
    double decompression_time_ms;
    double compression_ratio;
    double throughput_mbps;
} compression_benchmark_t;

int benchmark_compression_algorithms(const void* data, size_t size,
                                    compression_benchmark_t** results,
                                    size_t* count);

// =============================================================================
// PROTOCOL COMPARISON (gRPC vs REST)
// =============================================================================

typedef enum {
    PROTOCOL_REST_HTTP1,
    PROTOCOL_REST_HTTP2,
    PROTOCOL_GRPC,
    PROTOCOL_GRAPHQL,
    PROTOCOL_WEBSOCKET,
    PROTOCOL_MQTT,
    PROTOCOL_THRIFT_RPC
} network_protocol_t;

typedef struct {
    network_protocol_t protocol;
    serialization_format_t serialization;
    bool use_compression;
    bool use_tls;
    bool enable_multiplexing;    // HTTP/2, gRPC
    bool enable_streaming;       // gRPC, WebSocket
    uint32_t max_concurrent_streams;
} protocol_config_t;

typedef struct {
    network_protocol_t protocol;
    double avg_latency_ms;
    double p95_latency_ms;
    double p99_latency_ms;
    uint64_t requests_per_second;
    size_t avg_payload_size;
    size_t avg_wire_size;
    double overhead_bytes;
    double overhead_percentage;
    double cpu_usage;
    double memory_usage_mb;
} protocol_benchmark_t;

// Protocol handler
protocol_handler_t* protocol_handler_create(protocol_config_t* config);
void protocol_handler_destroy(protocol_handler_t* handler);

// Request/Response
typedef struct {
    char* endpoint;
    char* method;
    void* body;
    size_t body_size;
    char** headers;
    size_t header_count;
} protocol_request_t;

typedef struct {
    int status_code;
    void* body;
    size_t body_size;
    char** headers;
    size_t header_count;
    double latency_ms;
} protocol_response_t;

protocol_response_t* protocol_send_request(protocol_handler_t* handler,
                                           protocol_request_t* request);
void protocol_response_destroy(protocol_response_t* response);

// Protocol comparison
int compare_protocols(protocol_request_t* request,
                     protocol_config_t** configs, size_t config_count,
                     protocol_benchmark_t** results, size_t* result_count);

// =============================================================================
// CONNECTION MANAGEMENT & KEEP-ALIVE
// =============================================================================

typedef enum {
    CONN_STATUS_IDLE,
    CONN_STATUS_ACTIVE,
    CONN_STATUS_CLOSING,
    CONN_STATUS_CLOSED,
    CONN_STATUS_ERROR
} connection_status_t;

typedef struct {
    char id[64];
    char remote_host[256];
    int remote_port;
    connection_status_t status;
    uint64_t created_at;
    uint64_t last_used_at;
    uint64_t requests_sent;
    uint64_t bytes_sent;
    uint64_t bytes_received;
    bool is_persistent;
    bool is_multiplexed;
} connection_info_t;

typedef struct {
    size_t max_connections;
    size_t max_connections_per_host;
    uint64_t idle_timeout_ms;
    uint64_t connection_timeout_ms;
    uint64_t keep_alive_interval_ms;
    bool enable_keep_alive;
    bool enable_tcp_nodelay;
    bool enable_connection_reuse;
    size_t max_requests_per_connection;
} connection_pool_config_t;

typedef struct {
    size_t total_connections;
    size_t active_connections;
    size_t idle_connections;
    size_t reused_connections;
    uint64_t connection_reuse_rate;
    double avg_connection_lifetime_ms;
    uint64_t total_bytes_sent;
    uint64_t total_bytes_received;
} connection_pool_stats_t;

// Connection pool management
connection_manager_t* connection_manager_create(connection_pool_config_t* config);
void connection_manager_destroy(connection_manager_t* manager);

// Connection operations
int connection_acquire(connection_manager_t* manager, const char* host, int port,
                      int* conn_fd);
int connection_release(connection_manager_t* manager, int conn_fd);
int connection_close(connection_manager_t* manager, int conn_fd);

// Keep-alive management
int connection_send_keepalive(connection_manager_t* manager, int conn_fd);
int connection_check_alive(connection_manager_t* manager, int conn_fd);
int connection_close_idle(connection_manager_t* manager);

// Connection info
int connection_get_info(connection_manager_t* manager, int conn_fd,
                       connection_info_t* info);
int connection_get_all_info(connection_manager_t* manager,
                           connection_info_t** connections, size_t* count);
int connection_get_stats(connection_manager_t* manager,
                        connection_pool_stats_t* stats);

// =============================================================================
// HTTP/2 & MULTIPLEXING
// =============================================================================

typedef struct {
    uint32_t stream_id;
    char* path;
    int priority;
    bool is_active;
} http2_stream_t;

typedef struct {
    bool enable_server_push;
    size_t max_concurrent_streams;
    size_t initial_window_size;
    size_t max_frame_size;
    size_t max_header_list_size;
} http2_config_t;

// HTTP/2 stream management
int http2_create_stream(connection_manager_t* manager, int conn_fd,
                       const char* path, uint32_t* stream_id);
int http2_close_stream(connection_manager_t* manager, int conn_fd,
                      uint32_t stream_id);
int http2_get_streams(connection_manager_t* manager, int conn_fd,
                     http2_stream_t** streams, size_t* count);

// =============================================================================
// NETWORK OPTIMIZATION
// =============================================================================

typedef struct {
    bool enable_nagle_algorithm;     // TCP_NODELAY
    size_t send_buffer_size;
    size_t receive_buffer_size;
    bool enable_tcp_quickack;
    bool enable_tcp_fastopen;
    int tcp_keepalive_time;
    int tcp_keepalive_interval;
    int tcp_keepalive_probes;
} tcp_optimization_config_t;

// TCP optimization
int tcp_optimize_socket(int socket_fd, tcp_optimization_config_t* config);
int tcp_get_socket_info(int socket_fd, char* buffer, size_t size);

// Network performance testing
typedef struct {
    double bandwidth_mbps;
    double latency_ms;
    double jitter_ms;
    double packet_loss_rate;
    uint64_t bytes_transferred;
    double duration_ms;
} network_perf_result_t;

network_perf_result_t* test_network_performance(const char* host, int port,
                                                size_t data_size,
                                                size_t iterations);

// =============================================================================
// BATCH & STREAMING
// =============================================================================

// Batch request/response
typedef struct {
    protocol_request_t** requests;
    size_t request_count;
    bool parallel;
} batch_request_t;

typedef struct {
    protocol_response_t** responses;
    size_t response_count;
    double total_time_ms;
} batch_response_t;

batch_response_t* protocol_send_batch(protocol_handler_t* handler,
                                      batch_request_t* batch);
void batch_response_destroy(batch_response_t* response);

// Streaming
typedef void (*stream_callback_fn)(const void* data, size_t size, void* ctx);

typedef struct stream stream_t;

stream_t* stream_create(protocol_handler_t* handler, const char* endpoint);
void stream_destroy(stream_t* stream);
int stream_send(stream_t* stream, const void* data, size_t size);
int stream_receive(stream_t* stream, stream_callback_fn callback, void* ctx);
int stream_close(stream_t* stream);

// =============================================================================
// SERIALIZATION BENCHMARKING
// =============================================================================

typedef struct {
    serialization_format_t format;
    size_t serialized_size;
    double serialize_time_ms;
    double deserialize_time_ms;
    double total_time_ms;
    size_t throughput_mbps;
} format_benchmark_t;

int benchmark_serialization_formats(const void* object,
                                    format_benchmark_t** results,
                                    size_t* count);

// End-to-end protocol benchmark
typedef struct {
    network_protocol_t protocol;
    serialization_format_t format;
    compression_algorithm_t compression;
    double total_latency_ms;
    double serialization_time_ms;
    double compression_time_ms;
    double network_time_ms;
    double decompression_time_ms;
    double deserialization_time_ms;
    size_t original_size;
    size_t wire_size;
} end_to_end_benchmark_t;

end_to_end_benchmark_t* benchmark_end_to_end(const char* host, int port,
                                             const void* payload, size_t size,
                                             protocol_config_t* config);

#endif // NETWORK_SERIALIZATION_H
