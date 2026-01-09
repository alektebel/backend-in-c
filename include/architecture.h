#ifndef ARCHITECTURE_H
#define ARCHITECTURE_H

#include "common.h"

// ============================================================================
// Monolithic Architecture
// ============================================================================

typedef struct monolith_app monolith_app_t;
typedef struct monolith_module monolith_module_t;

monolith_app_t* monolith_app_create(const char* app_name);
void monolith_app_destroy(monolith_app_t* app);
int monolith_app_register_module(monolith_app_t* app, monolith_module_t* module);
int monolith_app_start(monolith_app_t* app);
int monolith_app_stop(monolith_app_t* app);

// Module management
monolith_module_t* monolith_module_create(const char* name,
                                         int (*init)(void*),
                                         int (*shutdown)(void*),
                                         void* user_data);
void monolith_module_destroy(monolith_module_t* module);

// ============================================================================
// Microservices Architecture
// ============================================================================

typedef struct microservice microservice_t;
typedef struct service_mesh service_mesh_t;

// Microservice operations
microservice_t* microservice_create(const char* name, const char* version);
void microservice_destroy(microservice_t* service);
int microservice_start(microservice_t* service, int port);
int microservice_stop(microservice_t* service);
int microservice_register_endpoint(microservice_t* service, const char* path,
                                   const char* method,
                                   int (*handler)(void*, void*));
int microservice_health_check(microservice_t* service);
char* microservice_get_metrics(microservice_t* service);

// Service mesh
service_mesh_t* service_mesh_create(void);
void service_mesh_destroy(service_mesh_t* mesh);
int service_mesh_register_service(service_mesh_t* mesh, microservice_t* service);
int service_mesh_discover_service(service_mesh_t* mesh, const char* service_name,
                                 char** host, int* port);
int service_mesh_route_request(service_mesh_t* mesh, const char* service_name,
                               const void* request, void** response);

// ============================================================================
// Load Balancing
// ============================================================================

typedef enum {
    LB_ALGORITHM_ROUND_ROBIN,
    LB_ALGORITHM_LEAST_CONNECTIONS,
    LB_ALGORITHM_WEIGHTED_ROUND_ROBIN,
    LB_ALGORITHM_IP_HASH,
    LB_ALGORITHM_RANDOM,
    LB_ALGORITHM_LEAST_RESPONSE_TIME
} lb_algorithm_t;

typedef struct load_balancer load_balancer_t;
typedef struct backend_server backend_server_t;

// Load balancer operations
load_balancer_t* load_balancer_create(lb_algorithm_t algorithm);
void load_balancer_destroy(load_balancer_t* lb);
int load_balancer_add_backend(load_balancer_t* lb, const char* host, int port,
                              int weight);
int load_balancer_remove_backend(load_balancer_t* lb, const char* host, int port);
backend_server_t* load_balancer_select_backend(load_balancer_t* lb);
int load_balancer_mark_unhealthy(load_balancer_t* lb, backend_server_t* server);
int load_balancer_mark_healthy(load_balancer_t* lb, backend_server_t* server);

// Backend server operations
const char* backend_server_get_host(const backend_server_t* server);
int backend_server_get_port(const backend_server_t* server);
int backend_server_is_healthy(const backend_server_t* server);
int backend_server_get_connections(const backend_server_t* server);

// Health checking
typedef struct health_checker health_checker_t;

health_checker_t* health_checker_create(uint64_t interval_ms);
void health_checker_destroy(health_checker_t* checker);
int health_checker_add_backend(health_checker_t* checker, backend_server_t* server,
                              int (*check_func)(backend_server_t*));
int health_checker_start(health_checker_t* checker);
int health_checker_stop(health_checker_t* checker);

// ============================================================================
// API Gateway
// ============================================================================

typedef struct api_gateway api_gateway_t;
typedef struct gateway_route gateway_route_t;

api_gateway_t* api_gateway_create(void);
void api_gateway_destroy(api_gateway_t* gateway);
int api_gateway_add_route(api_gateway_t* gateway, const char* path,
                         const char* target_service, const char* target_path);
int api_gateway_add_middleware(api_gateway_t* gateway,
                               int (*middleware)(void*, void*, void*));
int api_gateway_start(api_gateway_t* gateway, int port);
int api_gateway_stop(api_gateway_t* gateway);

// ============================================================================
// Reverse Proxy
// ============================================================================

typedef struct reverse_proxy reverse_proxy_t;

reverse_proxy_t* reverse_proxy_create(void);
void reverse_proxy_destroy(reverse_proxy_t* proxy);
int reverse_proxy_add_upstream(reverse_proxy_t* proxy, const char* name,
                               const char* host, int port);
int reverse_proxy_set_load_balancer(reverse_proxy_t* proxy, load_balancer_t* lb);
int reverse_proxy_start(reverse_proxy_t* proxy, int port);
int reverse_proxy_stop(reverse_proxy_t* proxy);

// ============================================================================
// Service Discovery
// ============================================================================

typedef struct service_discovery service_discovery_t;

typedef enum {
    DISCOVERY_CLIENT_SIDE,
    DISCOVERY_SERVER_SIDE
} discovery_type_t;

service_discovery_t* service_discovery_create(discovery_type_t type,
                                              const char* registry_url);
void service_discovery_destroy(service_discovery_t* discovery);
int service_discovery_register(service_discovery_t* discovery, const char* service_name,
                              const char* host, int port, 
                              const char** tags, size_t tag_count);
int service_discovery_deregister(service_discovery_t* discovery, 
                                const char* service_id);
char** service_discovery_find(service_discovery_t* discovery, const char* service_name,
                             size_t* count);
int service_discovery_watch(service_discovery_t* discovery, const char* service_name,
                           void (*callback)(const char*, int, void*),
                           void* user_data);

// ============================================================================
// Event-Driven Architecture
// ============================================================================

typedef struct event_bus event_bus_t;
typedef struct event event_t;

event_bus_t* event_bus_create(void);
void event_bus_destroy(event_bus_t* bus);
int event_bus_subscribe(event_bus_t* bus, const char* event_type,
                       void (*handler)(const event_t*, void*),
                       void* user_data);
int event_bus_unsubscribe(event_bus_t* bus, const char* event_type);
int event_bus_publish(event_bus_t* bus, const event_t* event);

// Event operations
event_t* event_create(const char* type, const void* data, size_t data_len);
void event_destroy(event_t* event);
const char* event_get_type(const event_t* event);
const void* event_get_data(const event_t* event, size_t* data_len);
uint64_t event_get_timestamp(const event_t* event);

// ============================================================================
// CQRS (Command Query Responsibility Segregation)
// ============================================================================

typedef struct command_handler command_handler_t;
typedef struct query_handler query_handler_t;
typedef struct cqrs_system cqrs_system_t;

cqrs_system_t* cqrs_system_create(void);
void cqrs_system_destroy(cqrs_system_t* system);
int cqrs_register_command(cqrs_system_t* system, const char* command_name,
                         int (*handler)(const void*, void*));
int cqrs_register_query(cqrs_system_t* system, const char* query_name,
                       int (*handler)(const void*, void**));
int cqrs_execute_command(cqrs_system_t* system, const char* command_name,
                        const void* command_data);
int cqrs_execute_query(cqrs_system_t* system, const char* query_name,
                      const void* query_data, void** result);

#endif // ARCHITECTURE_H
