#define _POSIX_C_SOURCE 200809L
#include "distributed.h"
#include <pthread.h>

// Service registry implementation
typedef struct service_node {
    service_info_t* info;
    struct service_node* next;
} service_node_t;

struct service_registry {
    service_node_t* services;
    pthread_rwlock_t lock;
    uint64_t next_id;
};

service_registry_t* service_registry_create(void) {
    service_registry_t* registry = safe_calloc(1, sizeof(service_registry_t));
    pthread_rwlock_init(&registry->lock, NULL);
    registry->next_id = 1;
    return registry;
}

void service_registry_destroy(service_registry_t* registry) {
    if (!registry) return;
    
    pthread_rwlock_wrlock(&registry->lock);
    
    service_node_t* current = registry->services;
    while (current) {
        service_node_t* next = current->next;
        service_info_destroy(current->info);
        safe_free((void**)&current);
        current = next;
    }
    
    pthread_rwlock_unlock(&registry->lock);
    pthread_rwlock_destroy(&registry->lock);
    safe_free((void**)&registry);
}

int service_register(service_registry_t* registry, const char* service_name, 
                     const char* host, int port) {
    if (!registry || !service_name || !host) {
        return ERROR_INVALID_PARAM;
    }
    
    service_info_t* info = safe_calloc(1, sizeof(service_info_t));
    info->service_id = safe_malloc(64);
    snprintf(info->service_id, 64, "service-%lu", registry->next_id++);
    info->service_name = safe_strdup(service_name);
    info->host = safe_strdup(host);
    info->port = port;
    info->last_heartbeat = get_timestamp_ms();
    info->healthy = 1;
    
    service_node_t* node = safe_calloc(1, sizeof(service_node_t));
    node->info = info;
    
    pthread_rwlock_wrlock(&registry->lock);
    node->next = registry->services;
    registry->services = node;
    pthread_rwlock_unlock(&registry->lock);
    
    return SUCCESS;
}

int service_deregister(service_registry_t* registry, const char* service_id) {
    if (!registry || !service_id) {
        return ERROR_INVALID_PARAM;
    }
    
    pthread_rwlock_wrlock(&registry->lock);
    
    service_node_t* current = registry->services;
    service_node_t* prev = NULL;
    
    while (current) {
        if (strcmp(current->info->service_id, service_id) == 0) {
            if (prev) {
                prev->next = current->next;
            } else {
                registry->services = current->next;
            }
            
            service_info_destroy(current->info);
            safe_free((void**)&current);
            pthread_rwlock_unlock(&registry->lock);
            return SUCCESS;
        }
        prev = current;
        current = current->next;
    }
    
    pthread_rwlock_unlock(&registry->lock);
    return ERROR_NOT_FOUND;
}

int service_heartbeat(service_registry_t* registry, const char* service_id) {
    if (!registry || !service_id) {
        return ERROR_INVALID_PARAM;
    }
    
    pthread_rwlock_wrlock(&registry->lock);
    
    service_node_t* current = registry->services;
    while (current) {
        if (strcmp(current->info->service_id, service_id) == 0) {
            current->info->last_heartbeat = get_timestamp_ms();
            current->info->healthy = 1;
            pthread_rwlock_unlock(&registry->lock);
            return SUCCESS;
        }
        current = current->next;
    }
    
    pthread_rwlock_unlock(&registry->lock);
    return ERROR_NOT_FOUND;
}

service_info_t* service_discover(service_registry_t* registry, const char* service_name) {
    if (!registry || !service_name) {
        return NULL;
    }
    
    pthread_rwlock_rdlock(&registry->lock);
    
    service_node_t* current = registry->services;
    while (current) {
        if (strcmp(current->info->service_name, service_name) == 0 && 
            current->info->healthy) {
            // Create a copy
            service_info_t* info = safe_calloc(1, sizeof(service_info_t));
            info->service_id = safe_strdup(current->info->service_id);
            info->service_name = safe_strdup(current->info->service_name);
            info->host = safe_strdup(current->info->host);
            info->port = current->info->port;
            info->last_heartbeat = current->info->last_heartbeat;
            info->healthy = current->info->healthy;
            
            pthread_rwlock_unlock(&registry->lock);
            return info;
        }
        current = current->next;
    }
    
    pthread_rwlock_unlock(&registry->lock);
    return NULL;
}

void service_info_destroy(service_info_t* info) {
    if (!info) return;
    safe_free((void**)&info->service_id);
    safe_free((void**)&info->service_name);
    safe_free((void**)&info->host);
    safe_free((void**)&info);
}

// Authentication implementation
struct auth_token {
    char* user_id;
    uint64_t created_at;
    uint64_t ttl_seconds;
};

auth_token_t* auth_token_create(const char* user_id, uint64_t ttl_seconds) {
    if (!user_id) return NULL;
    
    auth_token_t* token = safe_calloc(1, sizeof(auth_token_t));
    token->user_id = safe_strdup(user_id);
    token->created_at = get_timestamp_ms();
    token->ttl_seconds = ttl_seconds;
    return token;
}

void auth_token_destroy(auth_token_t* token) {
    if (!token) return;
    safe_free((void**)&token->user_id);
    safe_free((void**)&token);
}

int auth_token_validate(const auth_token_t* token) {
    if (!token) return 0;
    
    uint64_t now = get_timestamp_ms();
    uint64_t age_ms = now - token->created_at;
    uint64_t ttl_ms = token->ttl_seconds * 1000;
    
    return age_ms < ttl_ms;
}

char* auth_token_serialize(const auth_token_t* token) {
    if (!token) return NULL;
    
    char* str = safe_malloc(256);
    snprintf(str, 256, "%s:%lu:%lu", token->user_id, token->created_at, token->ttl_seconds);
    return str;
}

auth_token_t* auth_token_deserialize(const char* token_str) {
    if (!token_str || strlen(token_str) > 256) return NULL;
    
    char user_id[128];
    uint64_t created_at, ttl_seconds;
    
    // sscanf with %127[^:] is safe as it limits to 127 chars + null terminator
    if (sscanf(token_str, "%127[^:]:%lu:%lu", user_id, &created_at, &ttl_seconds) != 3) {
        return NULL;
    }
    
    auth_token_t* token = safe_calloc(1, sizeof(auth_token_t));
    token->user_id = safe_strdup(user_id);
    token->created_at = created_at;
    token->ttl_seconds = ttl_seconds;
    return token;
}

const char* auth_token_get_user_id(const auth_token_t* token) {
    return token ? token->user_id : NULL;
}

// Rate limiter implementation
typedef struct rate_limit_entry {
    char* client_id;
    int request_count;
    uint64_t window_start;
    struct rate_limit_entry* next;
} rate_limit_entry_t;

struct rate_limiter {
    int max_requests;
    uint64_t window_ms;
    rate_limit_entry_t* entries;
    pthread_mutex_t lock;
};

rate_limiter_t* rate_limiter_create(int max_requests, uint64_t window_ms) {
    rate_limiter_t* limiter = safe_calloc(1, sizeof(rate_limiter_t));
    limiter->max_requests = max_requests;
    limiter->window_ms = window_ms;
    pthread_mutex_init(&limiter->lock, NULL);
    return limiter;
}

void rate_limiter_destroy(rate_limiter_t* limiter) {
    if (!limiter) return;
    
    rate_limit_entry_t* current = limiter->entries;
    while (current) {
        rate_limit_entry_t* next = current->next;
        safe_free((void**)&current->client_id);
        safe_free((void**)&current);
        current = next;
    }
    
    pthread_mutex_destroy(&limiter->lock);
    safe_free((void**)&limiter);
}

int rate_limiter_allow(rate_limiter_t* limiter, const char* client_id) {
    if (!limiter || !client_id) return 0;
    
    pthread_mutex_lock(&limiter->lock);
    
    uint64_t now = get_timestamp_ms();
    
    // Find or create entry
    rate_limit_entry_t* entry = limiter->entries;
    while (entry) {
        if (strcmp(entry->client_id, client_id) == 0) {
            break;
        }
        entry = entry->next;
    }
    
    if (!entry) {
        entry = safe_calloc(1, sizeof(rate_limit_entry_t));
        entry->client_id = safe_strdup(client_id);
        entry->window_start = now;
        entry->request_count = 0;
        entry->next = limiter->entries;
        limiter->entries = entry;
    }
    
    // Check if window has expired
    if (now - entry->window_start > limiter->window_ms) {
        entry->window_start = now;
        entry->request_count = 0;
    }
    
    // Check rate limit
    if (entry->request_count >= limiter->max_requests) {
        pthread_mutex_unlock(&limiter->lock);
        return 0;  // Rate limit exceeded
    }
    
    entry->request_count++;
    pthread_mutex_unlock(&limiter->lock);
    return 1;  // Request allowed
}

int rate_limiter_reset(rate_limiter_t* limiter, const char* client_id) {
    if (!limiter || !client_id) return ERROR_INVALID_PARAM;
    
    pthread_mutex_lock(&limiter->lock);
    
    rate_limit_entry_t* entry = limiter->entries;
    rate_limit_entry_t* prev = NULL;
    
    while (entry) {
        if (strcmp(entry->client_id, client_id) == 0) {
            if (prev) {
                prev->next = entry->next;
            } else {
                limiter->entries = entry->next;
            }
            
            safe_free((void**)&entry->client_id);
            safe_free((void**)&entry);
            pthread_mutex_unlock(&limiter->lock);
            return SUCCESS;
        }
        prev = entry;
        entry = entry->next;
    }
    
    pthread_mutex_unlock(&limiter->lock);
    return ERROR_NOT_FOUND;
}

// Circuit breaker implementation
struct circuit_breaker {
    circuit_state_t state;
    int failure_count;
    int failure_threshold;
    uint64_t timeout_ms;
    uint64_t last_failure_time;
    pthread_mutex_t lock;
};

circuit_breaker_t* circuit_breaker_create(int failure_threshold, uint64_t timeout_ms) {
    circuit_breaker_t* breaker = safe_calloc(1, sizeof(circuit_breaker_t));
    breaker->state = CIRCUIT_CLOSED;
    breaker->failure_threshold = failure_threshold;
    breaker->timeout_ms = timeout_ms;
    pthread_mutex_init(&breaker->lock, NULL);
    return breaker;
}

void circuit_breaker_destroy(circuit_breaker_t* breaker) {
    if (!breaker) return;
    pthread_mutex_destroy(&breaker->lock);
    safe_free((void**)&breaker);
}

int circuit_breaker_call(circuit_breaker_t* breaker, int (*func)(void*), void* arg) {
    if (!breaker || !func) return ERROR_INVALID_PARAM;
    
    pthread_mutex_lock(&breaker->lock);
    
    uint64_t now = get_timestamp_ms();
    
    // Check if circuit should move from OPEN to HALF_OPEN
    if (breaker->state == CIRCUIT_OPEN) {
        if (now - breaker->last_failure_time > breaker->timeout_ms) {
            breaker->state = CIRCUIT_HALF_OPEN;
        } else {
            pthread_mutex_unlock(&breaker->lock);
            return ERROR_TIMEOUT;  // Circuit is open
        }
    }
    
    pthread_mutex_unlock(&breaker->lock);
    
    // Execute function
    int result = func(arg);
    
    if (result == SUCCESS) {
        circuit_breaker_record_success(breaker);
    } else {
        circuit_breaker_record_failure(breaker);
    }
    
    return result;
}

void circuit_breaker_record_success(circuit_breaker_t* breaker) {
    if (!breaker) return;
    
    pthread_mutex_lock(&breaker->lock);
    
    if (breaker->state == CIRCUIT_HALF_OPEN) {
        breaker->state = CIRCUIT_CLOSED;
        breaker->failure_count = 0;
    }
    
    pthread_mutex_unlock(&breaker->lock);
}

void circuit_breaker_record_failure(circuit_breaker_t* breaker) {
    if (!breaker) return;
    
    pthread_mutex_lock(&breaker->lock);
    
    breaker->failure_count++;
    breaker->last_failure_time = get_timestamp_ms();
    
    if (breaker->failure_count >= breaker->failure_threshold) {
        breaker->state = CIRCUIT_OPEN;
    }
    
    pthread_mutex_unlock(&breaker->lock);
}

circuit_state_t circuit_breaker_get_state(circuit_breaker_t* breaker) {
    if (!breaker) return CIRCUIT_OPEN;
    
    pthread_mutex_lock(&breaker->lock);
    circuit_state_t state = breaker->state;
    pthread_mutex_unlock(&breaker->lock);
    
    return state;
}

// Retry policy implementation
struct retry_policy {
    int max_retries;
    uint64_t initial_delay_ms;
    double backoff_multiplier;
};

retry_policy_t* retry_policy_create(int max_retries, uint64_t initial_delay_ms, 
                                    double backoff_multiplier) {
    retry_policy_t* policy = safe_calloc(1, sizeof(retry_policy_t));
    policy->max_retries = max_retries;
    policy->initial_delay_ms = initial_delay_ms;
    policy->backoff_multiplier = backoff_multiplier;
    return policy;
}

void retry_policy_destroy(retry_policy_t* policy) {
    safe_free((void**)&policy);
}

int retry_with_policy(retry_policy_t* policy, int (*func)(void*), void* arg) {
    if (!policy || !func) return ERROR_INVALID_PARAM;
    
    int result;
    uint64_t delay_ms = policy->initial_delay_ms;
    
    for (int attempt = 0; attempt <= policy->max_retries; attempt++) {
        result = func(arg);
        
        if (result == SUCCESS) {
            return SUCCESS;
        }
        
        if (attempt < policy->max_retries) {
            struct timespec ts;
            ts.tv_sec = delay_ms / 1000;
            ts.tv_nsec = (delay_ms % 1000) * 1000000;
            nanosleep(&ts, NULL);
            
            delay_ms = (uint64_t)(delay_ms * policy->backoff_multiplier);
        }
    }
    
    return result;
}

// Metrics collector implementation
typedef struct metric {
    char* name;
    double value;
    struct metric* next;
} metric_t;

struct metrics_collector {
    metric_t* metrics;
    pthread_mutex_t lock;
};

metrics_collector_t* metrics_collector_create(void) {
    metrics_collector_t* collector = safe_calloc(1, sizeof(metrics_collector_t));
    pthread_mutex_init(&collector->lock, NULL);
    return collector;
}

void metrics_collector_destroy(metrics_collector_t* collector) {
    if (!collector) return;
    
    metric_t* current = collector->metrics;
    while (current) {
        metric_t* next = current->next;
        safe_free((void**)&current->name);
        safe_free((void**)&current);
        current = next;
    }
    
    pthread_mutex_destroy(&collector->lock);
    safe_free((void**)&collector);
}

static metric_t* find_or_create_metric(metrics_collector_t* collector, const char* name) {
    metric_t* metric = collector->metrics;
    while (metric) {
        if (strcmp(metric->name, name) == 0) {
            return metric;
        }
        metric = metric->next;
    }
    
    // Create new metric
    metric = safe_calloc(1, sizeof(metric_t));
    metric->name = safe_strdup(name);
    metric->next = collector->metrics;
    collector->metrics = metric;
    return metric;
}

int metrics_counter_inc(metrics_collector_t* collector, const char* name) {
    if (!collector || !name) return ERROR_INVALID_PARAM;
    
    pthread_mutex_lock(&collector->lock);
    metric_t* metric = find_or_create_metric(collector, name);
    metric->value += 1.0;
    pthread_mutex_unlock(&collector->lock);
    
    return SUCCESS;
}

int metrics_gauge_set(metrics_collector_t* collector, const char* name, double value) {
    if (!collector || !name) return ERROR_INVALID_PARAM;
    
    pthread_mutex_lock(&collector->lock);
    metric_t* metric = find_or_create_metric(collector, name);
    metric->value = value;
    pthread_mutex_unlock(&collector->lock);
    
    return SUCCESS;
}

int metrics_histogram_observe(metrics_collector_t* collector, const char* name, double value) {
    if (!collector || !name) return ERROR_INVALID_PARAM;
    
    pthread_mutex_lock(&collector->lock);
    metric_t* metric = find_or_create_metric(collector, name);
    // Simple average for histogram
    metric->value = (metric->value + value) / 2.0;
    pthread_mutex_unlock(&collector->lock);
    
    return SUCCESS;
}

char* metrics_export(metrics_collector_t* collector) {
    if (!collector) return NULL;
    
    size_t size = 4096;
    char* buffer = safe_malloc(size);
    size_t offset = 0;
    
    pthread_mutex_lock(&collector->lock);
    
    metric_t* metric = collector->metrics;
    while (metric) {
        offset += snprintf(buffer + offset, size - offset, "%s: %.2f\n", 
                          metric->name, metric->value);
        metric = metric->next;
    }
    
    pthread_mutex_unlock(&collector->lock);
    
    return buffer;
}
