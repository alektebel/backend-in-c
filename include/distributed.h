#ifndef DISTRIBUTED_H
#define DISTRIBUTED_H

#include "common.h"

// Service discovery
typedef struct service_registry service_registry_t;
typedef struct service_info service_info_t;

struct service_info {
    char* service_id;
    char* service_name;
    char* host;
    int port;
    uint64_t last_heartbeat;
    int healthy;
};

service_registry_t* service_registry_create(void);
void service_registry_destroy(service_registry_t* registry);
int service_register(service_registry_t* registry, const char* service_name, 
                     const char* host, int port);
int service_deregister(service_registry_t* registry, const char* service_id);
int service_heartbeat(service_registry_t* registry, const char* service_id);
service_info_t* service_discover(service_registry_t* registry, const char* service_name);
void service_info_destroy(service_info_t* info);

// Authentication
typedef struct auth_token auth_token_t;

auth_token_t* auth_token_create(const char* user_id, uint64_t ttl_seconds);
void auth_token_destroy(auth_token_t* token);
int auth_token_validate(const auth_token_t* token);
char* auth_token_serialize(const auth_token_t* token);
auth_token_t* auth_token_deserialize(const char* token_str);

// Rate limiting
typedef struct rate_limiter rate_limiter_t;

rate_limiter_t* rate_limiter_create(int max_requests, uint64_t window_ms);
void rate_limiter_destroy(rate_limiter_t* limiter);
int rate_limiter_allow(rate_limiter_t* limiter, const char* client_id);
int rate_limiter_reset(rate_limiter_t* limiter, const char* client_id);

// Circuit breaker
typedef enum {
    CIRCUIT_CLOSED,
    CIRCUIT_OPEN,
    CIRCUIT_HALF_OPEN
} circuit_state_t;

typedef struct circuit_breaker circuit_breaker_t;

circuit_breaker_t* circuit_breaker_create(int failure_threshold, uint64_t timeout_ms);
void circuit_breaker_destroy(circuit_breaker_t* breaker);
int circuit_breaker_call(circuit_breaker_t* breaker, int (*func)(void*), void* arg);
void circuit_breaker_record_success(circuit_breaker_t* breaker);
void circuit_breaker_record_failure(circuit_breaker_t* breaker);
circuit_state_t circuit_breaker_get_state(circuit_breaker_t* breaker);

// Retry policy
typedef struct retry_policy retry_policy_t;

retry_policy_t* retry_policy_create(int max_retries, uint64_t initial_delay_ms, 
                                    double backoff_multiplier);
void retry_policy_destroy(retry_policy_t* policy);
int retry_with_policy(retry_policy_t* policy, int (*func)(void*), void* arg);

// Observability - Metrics
typedef struct metrics_collector metrics_collector_t;

metrics_collector_t* metrics_collector_create(void);
void metrics_collector_destroy(metrics_collector_t* collector);
int metrics_counter_inc(metrics_collector_t* collector, const char* name);
int metrics_gauge_set(metrics_collector_t* collector, const char* name, double value);
int metrics_histogram_observe(metrics_collector_t* collector, const char* name, double value);
char* metrics_export(metrics_collector_t* collector);

#endif // DISTRIBUTED_H
