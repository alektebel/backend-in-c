#ifndef CONCURRENCY_H
#define CONCURRENCY_H

#include <pthread.h>
#include <stdint.h>
#include <stdbool.h>
#include "common.h"

// =============================================================================
// CONCURRENCY & THROUGHPUT
// =============================================================================
// This module provides templates for:
// - Thread pools for work distribution
// - Async I/O and event-driven architectures
// - Backpressure handling
// - Locking vs lock-free approaches
// - Contention hotspot detection and mitigation

// Forward declarations
typedef struct thread_pool thread_pool_t;
typedef struct async_io_context async_io_context_t;
typedef struct lock_free_queue lock_free_queue_t;
typedef struct contention_monitor contention_monitor_t;

// =============================================================================
// THREAD POOL
// =============================================================================

typedef enum {
    THREAD_POOL_FIXED,       // Fixed number of threads
    THREAD_POOL_CACHED,      // Create threads as needed, cache idle threads
    THREAD_POOL_WORK_STEALING // Work-stealing thread pool
} thread_pool_type_t;

typedef enum {
    TASK_PRIORITY_LOW,
    TASK_PRIORITY_NORMAL,
    TASK_PRIORITY_HIGH,
    TASK_PRIORITY_CRITICAL
} task_priority_t;

typedef enum {
    TASK_STATE_PENDING,
    TASK_STATE_RUNNING,
    TASK_STATE_COMPLETED,
    TASK_STATE_FAILED,
    TASK_STATE_CANCELLED
} task_state_t;

// Task function signature
typedef void (*task_fn)(void* arg);
typedef void (*task_completion_fn)(void* result, void* arg);

typedef struct {
    char id[64];
    task_fn function;
    void* arg;
    task_priority_t priority;
    task_state_t state;
    task_completion_fn on_complete;
    void* completion_arg;
    uint64_t submitted_at;
    uint64_t started_at;
    uint64_t completed_at;
} task_t;

typedef struct {
    thread_pool_type_t type;
    size_t min_threads;
    size_t max_threads;
    size_t queue_size;
    uint64_t thread_idle_timeout_ms;
    bool enable_work_stealing;
    bool enable_priority_queue;
} thread_pool_config_t;

typedef struct {
    size_t active_threads;
    size_t idle_threads;
    size_t total_threads;
    size_t queued_tasks;
    uint64_t completed_tasks;
    uint64_t failed_tasks;
    uint64_t rejected_tasks;
    double avg_task_time_ms;
    double avg_wait_time_ms;
    double thread_utilization;
} thread_pool_stats_t;

// Thread pool management
thread_pool_t* thread_pool_create(thread_pool_config_t* config);
void thread_pool_destroy(thread_pool_t* pool);
int thread_pool_start(thread_pool_t* pool);
int thread_pool_shutdown(thread_pool_t* pool, bool wait_for_completion);

// Task submission
task_t* thread_pool_submit(thread_pool_t* pool, task_fn function, void* arg);
task_t* thread_pool_submit_with_priority(thread_pool_t* pool, task_fn function,
                                         void* arg, task_priority_t priority);
int thread_pool_cancel_task(thread_pool_t* pool, task_t* task);
int thread_pool_wait_for_task(task_t* task);
int thread_pool_wait_all(thread_pool_t* pool);

// Thread pool statistics
int thread_pool_get_stats(thread_pool_t* pool, thread_pool_stats_t* stats);
int thread_pool_resize(thread_pool_t* pool, size_t new_size);

// =============================================================================
// ASYNC I/O & EVENT-DRIVEN ARCHITECTURE
// =============================================================================

typedef enum {
    ASYNC_IO_EPOLL,          // Linux epoll
    ASYNC_IO_KQUEUE,         // BSD kqueue
    ASYNC_IO_IOCP,           // Windows IOCP
    ASYNC_IO_SELECT,         // POSIX select (fallback)
    ASYNC_IO_IO_URING        // Linux io_uring (modern)
} async_io_backend_t;

typedef enum {
    IO_EVENT_READ,
    IO_EVENT_WRITE,
    IO_EVENT_ACCEPT,
    IO_EVENT_CONNECT,
    IO_EVENT_CLOSE,
    IO_EVENT_ERROR,
    IO_EVENT_TIMEOUT
} io_event_type_t;

typedef enum {
    IO_OP_PENDING,
    IO_OP_COMPLETED,
    IO_OP_FAILED,
    IO_OP_CANCELLED,
    IO_OP_TIMEOUT
} io_op_status_t;

// I/O callback signature
typedef void (*io_callback_fn)(int fd, io_event_type_t event, void* data);

typedef struct {
    int fd;
    io_event_type_t event;
    io_callback_fn callback;
    void* user_data;
    uint64_t timeout_ms;
} io_event_t;

typedef struct {
    async_io_backend_t backend;
    size_t max_events;
    uint64_t event_timeout_ms;
    size_t worker_threads;
    bool enable_edge_triggered;
} async_io_config_t;

typedef struct {
    uint64_t total_events;
    uint64_t read_events;
    uint64_t write_events;
    uint64_t error_events;
    uint64_t timeout_events;
    double avg_event_latency_ms;
    double events_per_second;
} async_io_stats_t;

// Async I/O context management
async_io_context_t* async_io_create(async_io_config_t* config);
void async_io_destroy(async_io_context_t* ctx);
int async_io_run(async_io_context_t* ctx);
int async_io_stop(async_io_context_t* ctx);

// Event registration
int async_io_register(async_io_context_t* ctx, int fd, io_event_type_t event,
                     io_callback_fn callback, void* user_data);
int async_io_unregister(async_io_context_t* ctx, int fd);
int async_io_modify(async_io_context_t* ctx, int fd, io_event_type_t event);

// Async operations
typedef struct async_operation async_operation_t;

async_operation_t* async_read(async_io_context_t* ctx, int fd, void* buffer,
                              size_t size, io_callback_fn callback, void* data);
async_operation_t* async_write(async_io_context_t* ctx, int fd, 
                               const void* buffer, size_t size,
                               io_callback_fn callback, void* data);
async_operation_t* async_accept(async_io_context_t* ctx, int listen_fd,
                                io_callback_fn callback, void* data);
async_operation_t* async_connect(async_io_context_t* ctx, const char* host,
                                 int port, io_callback_fn callback, void* data);

int async_operation_cancel(async_operation_t* op);
int async_operation_wait(async_operation_t* op);
io_op_status_t async_operation_status(async_operation_t* op);

// Statistics
int async_io_get_stats(async_io_context_t* ctx, async_io_stats_t* stats);

// =============================================================================
// BACKPRESSURE HANDLING
// =============================================================================

typedef enum {
    BACKPRESSURE_DROP,           // Drop new requests
    BACKPRESSURE_BLOCK,          // Block until capacity
    BACKPRESSURE_QUEUE,          // Queue with bounds
    BACKPRESSURE_REJECT,         // Reject with error
    BACKPRESSURE_ADAPTIVE        // Adaptive based on metrics
} backpressure_strategy_t;

typedef struct {
    backpressure_strategy_t strategy;
    size_t max_queue_size;
    uint64_t max_wait_time_ms;
    double high_water_mark;      // 0.0-1.0
    double low_water_mark;       // 0.0-1.0
    bool enable_circuit_breaker;
} backpressure_config_t;

typedef struct {
    size_t current_load;
    size_t max_load;
    double load_factor;          // 0.0-1.0
    uint64_t requests_accepted;
    uint64_t requests_rejected;
    uint64_t requests_dropped;
    bool is_throttling;
} backpressure_stats_t;

typedef struct backpressure_controller backpressure_controller_t;

// Backpressure control
backpressure_controller_t* backpressure_create(backpressure_config_t* config);
void backpressure_destroy(backpressure_controller_t* bp);

bool backpressure_can_accept(backpressure_controller_t* bp);
int backpressure_acquire(backpressure_controller_t* bp);
int backpressure_release(backpressure_controller_t* bp);
int backpressure_get_stats(backpressure_controller_t* bp, 
                          backpressure_stats_t* stats);

// Adaptive backpressure
int backpressure_update_metrics(backpressure_controller_t* bp,
                                double latency_ms, bool success);
int backpressure_adjust_limits(backpressure_controller_t* bp);

// =============================================================================
// LOCKING MECHANISMS
// =============================================================================

typedef enum {
    LOCK_TYPE_MUTEX,
    LOCK_TYPE_SPINLOCK,
    LOCK_TYPE_RWLOCK,
    LOCK_TYPE_RECURSIVE,
    LOCK_TYPE_ADAPTIVE
} lock_type_t;

typedef struct {
    lock_type_t type;
    char name[64];
    void* mutex_ptr;      // pthread_mutex_t
    void* rwlock_ptr;     // pthread_rwlock_t
    uint64_t acquisitions;
    uint64_t contentions;
    double avg_hold_time_ms;
    uint64_t max_wait_time_ms;
} lock_t;

typedef struct {
    uint64_t acquire_count;
    uint64_t contention_count;
    uint64_t wait_time_total_ms;
    double contention_rate;
    double avg_hold_time_ms;
    double max_hold_time_ms;
} lock_stats_t;

// Lock management
lock_t* lock_create(lock_type_t type, const char* name);
void lock_destroy(lock_t* lock);

int lock_acquire(lock_t* lock);
int lock_try_acquire(lock_t* lock);
int lock_release(lock_t* lock);

// Read-write locks
int lock_read_acquire(lock_t* lock);
int lock_write_acquire(lock_t* lock);
int lock_read_release(lock_t* lock);
int lock_write_release(lock_t* lock);

// Lock statistics
int lock_get_stats(lock_t* lock, lock_stats_t* stats);
int lock_reset_stats(lock_t* lock);

// =============================================================================
// LOCK-FREE DATA STRUCTURES
// =============================================================================

// Lock-free queue (MPSC - Multiple Producer Single Consumer)
typedef struct lock_free_queue_node {
    void* data;
    struct lock_free_queue_node* next;
} lock_free_queue_node_t;

lock_free_queue_t* lock_free_queue_create(size_t capacity);
void lock_free_queue_destroy(lock_free_queue_t* queue);

int lock_free_queue_push(lock_free_queue_t* queue, void* data);
void* lock_free_queue_pop(lock_free_queue_t* queue);
bool lock_free_queue_is_empty(lock_free_queue_t* queue);
size_t lock_free_queue_size(lock_free_queue_t* queue);

// Lock-free stack
typedef struct lock_free_stack lock_free_stack_t;

lock_free_stack_t* lock_free_stack_create(void);
void lock_free_stack_destroy(lock_free_stack_t* stack);

int lock_free_stack_push(lock_free_stack_t* stack, void* data);
void* lock_free_stack_pop(lock_free_stack_t* stack);
bool lock_free_stack_is_empty(lock_free_stack_t* stack);

// Lock-free hash map
typedef struct lock_free_hashmap lock_free_hashmap_t;

lock_free_hashmap_t* lock_free_hashmap_create(size_t capacity);
void lock_free_hashmap_destroy(lock_free_hashmap_t* map);

int lock_free_hashmap_put(lock_free_hashmap_t* map, const char* key, void* value);
void* lock_free_hashmap_get(lock_free_hashmap_t* map, const char* key);
int lock_free_hashmap_remove(lock_free_hashmap_t* map, const char* key);
size_t lock_free_hashmap_size(lock_free_hashmap_t* map);

// Atomic operations
typedef struct {
    volatile uint64_t value;
} atomic_counter_t;

atomic_counter_t* atomic_counter_create(uint64_t initial_value);
void atomic_counter_destroy(atomic_counter_t* counter);
uint64_t atomic_counter_increment(atomic_counter_t* counter);
uint64_t atomic_counter_decrement(atomic_counter_t* counter);
uint64_t atomic_counter_add(atomic_counter_t* counter, uint64_t delta);
uint64_t atomic_counter_get(atomic_counter_t* counter);
void atomic_counter_set(atomic_counter_t* counter, uint64_t value);

// =============================================================================
// CONTENTION HOTSPOT DETECTION
// =============================================================================

typedef struct {
    char resource_name[128];
    char location[256];          // File:line
    uint64_t contention_count;
    double contention_rate;
    double avg_wait_time_ms;
    double max_wait_time_ms;
    uint64_t thread_count;       // Number of threads contending
    bool is_hotspot;
} contention_hotspot_t;

typedef struct {
    bool enable_tracking;
    double hotspot_threshold;    // Contention rate to flag as hotspot
    uint64_t sampling_interval_ms;
    size_t max_tracked_resources;
} contention_config_t;

// Contention monitoring
contention_monitor_t* contention_monitor_create(contention_config_t* config);
void contention_monitor_destroy(contention_monitor_t* monitor);

int contention_monitor_track_lock(contention_monitor_t* monitor, lock_t* lock);
int contention_monitor_record_wait(contention_monitor_t* monitor, 
                                   const char* resource, double wait_time_ms);

int contention_monitor_get_hotspots(contention_monitor_t* monitor,
                                    contention_hotspot_t** hotspots,
                                    size_t* count);
int contention_monitor_reset(contention_monitor_t* monitor);

// =============================================================================
// CONCURRENCY PATTERNS
// =============================================================================

// Producer-Consumer pattern
typedef struct producer_consumer producer_consumer_t;

producer_consumer_t* producer_consumer_create(size_t buffer_size);
void producer_consumer_destroy(producer_consumer_t* pc);

int producer_consumer_produce(producer_consumer_t* pc, void* item);
void* producer_consumer_consume(producer_consumer_t* pc);

// Reader-Writer pattern (with writer preference)
typedef struct reader_writer reader_writer_t;

reader_writer_t* reader_writer_create(void);
void reader_writer_destroy(reader_writer_t* rw);

int reader_writer_read_lock(reader_writer_t* rw);
int reader_writer_read_unlock(reader_writer_t* rw);
int reader_writer_write_lock(reader_writer_t* rw);
int reader_writer_write_unlock(reader_writer_t* rw);

// Barrier pattern (synchronization point)
typedef struct barrier barrier_t;

barrier_t* barrier_create(size_t thread_count);
void barrier_destroy(barrier_t* barrier);
int barrier_wait(barrier_t* barrier);

// Future/Promise pattern
typedef struct future future_t;

future_t* future_create(void);
void future_destroy(future_t* future);

int future_set(future_t* future, void* value);
void* future_get(future_t* future);
bool future_is_ready(future_t* future);
int future_wait(future_t* future, uint64_t timeout_ms);

// =============================================================================
// THROUGHPUT OPTIMIZATION
// =============================================================================

typedef struct {
    uint64_t requests_per_second;
    uint64_t transactions_per_second;
    double avg_response_time_ms;
    double p95_response_time_ms;
    double p99_response_time_ms;
    double cpu_utilization;
    double memory_utilization;
    size_t active_connections;
    size_t thread_count;
} throughput_metrics_t;

typedef struct throughput_optimizer throughput_optimizer_t;

throughput_optimizer_t* throughput_optimizer_create(void);
void throughput_optimizer_destroy(throughput_optimizer_t* optimizer);

int throughput_record_request(throughput_optimizer_t* optimizer, 
                              double duration_ms);
int throughput_get_metrics(throughput_optimizer_t* optimizer,
                          throughput_metrics_t* metrics);

// Optimization recommendations
typedef struct {
    char recommendation[512];
    double expected_improvement;
    int priority;
} throughput_recommendation_t;

int throughput_analyze(throughput_optimizer_t* optimizer,
                      throughput_recommendation_t** recommendations,
                      size_t* count);

// =============================================================================
// PARALLEL PROCESSING
// =============================================================================

// Parallel for loop
typedef void (*parallel_for_fn)(size_t index, void* arg);

int parallel_for(thread_pool_t* pool, size_t start, size_t end,
                parallel_for_fn function, void* arg);

// Map-Reduce pattern
typedef void* (*map_fn)(void* input, void* context);
typedef void* (*reduce_fn)(void* acc, void* value, void* context);

void* parallel_map_reduce(thread_pool_t* pool, void** inputs, size_t count,
                          map_fn map_func, reduce_fn reduce_func,
                          void* initial_value, void* context);

#endif // CONCURRENCY_H
