#ifndef CONTAINER_H
#define CONTAINER_H

#include "common.h"

// ============================================================================
// Container Runtime (Docker-like)
// ============================================================================

typedef struct container container_t;
typedef struct container_config container_config_t;
typedef struct container_image container_image_t;

typedef enum {
    CONTAINER_STATE_CREATED,
    CONTAINER_STATE_RUNNING,
    CONTAINER_STATE_PAUSED,
    CONTAINER_STATE_STOPPED,
    CONTAINER_STATE_RESTARTING
} container_state_t;

// Container configuration
struct container_config {
    const char* image;
    const char* name;
    const char** command;
    size_t command_count;
    const char** env_vars;
    size_t env_var_count;
    int memory_limit_mb;
    int cpu_shares;
    int privileged;
    const char** volumes;
    size_t volume_count;
    const char** port_mappings;  // Format: "host:container"
    size_t port_mapping_count;
};

// Container operations
container_t* container_create(const container_config_t* config);
void container_destroy(container_t* container);
int container_start(container_t* container);
int container_stop(container_t* container, uint64_t timeout_seconds);
int container_pause(container_t* container);
int container_unpause(container_t* container);
int container_restart(container_t* container);
int container_remove(container_t* container, int force);
container_state_t container_get_state(const container_t* container);
const char* container_get_id(const container_t* container);

// Container execution
int container_exec(container_t* container, const char** command, size_t command_count,
                  char** output);

// Container logs
char* container_get_logs(container_t* container, int follow);

// Container stats
typedef struct {
    double cpu_percent;
    uint64_t memory_usage_bytes;
    uint64_t memory_limit_bytes;
    uint64_t network_rx_bytes;
    uint64_t network_tx_bytes;
    uint64_t block_io_read_bytes;
    uint64_t block_io_write_bytes;
} container_stats_t;

int container_get_stats(container_t* container, container_stats_t* stats);

// ============================================================================
// Container Images
// ============================================================================

container_image_t* container_image_pull(const char* image_name);
int container_image_build(const char* dockerfile_path, const char* tag);
int container_image_push(const char* image_name);
int container_image_remove(const char* image_name);
char** container_image_list(size_t* count);
void container_image_destroy(container_image_t* image);

// ============================================================================
// Container Networking
// ============================================================================

typedef struct container_network container_network_t;

container_network_t* container_network_create(const char* name, const char* driver);
void container_network_destroy(container_network_t* network);
int container_network_connect(container_network_t* network, container_t* container);
int container_network_disconnect(container_network_t* network, container_t* container);
char** container_network_list(size_t* count);

// ============================================================================
// Container Volumes
// ============================================================================

typedef struct container_volume container_volume_t;

container_volume_t* container_volume_create(const char* name);
void container_volume_destroy(container_volume_t* volume);
int container_volume_remove(const char* name);
char** container_volume_list(size_t* count);

// ============================================================================
// Docker Compose (Multi-container applications)
// ============================================================================

typedef struct compose_project compose_project_t;
typedef struct compose_service compose_service_t;

compose_project_t* compose_project_create(const char* project_name);
void compose_project_destroy(compose_project_t* project);
int compose_project_load_file(compose_project_t* project, const char* compose_file);
int compose_project_up(compose_project_t* project);
int compose_project_down(compose_project_t* project);
int compose_project_start(compose_project_t* project);
int compose_project_stop(compose_project_t* project);
int compose_project_restart(compose_project_t* project);

// Service operations
int compose_project_scale_service(compose_project_t* project, 
                                 const char* service_name, int replicas);
compose_service_t* compose_project_get_service(compose_project_t* project,
                                              const char* service_name);

// ============================================================================
// Kubernetes-like Orchestration
// ============================================================================

typedef struct orchestrator orchestrator_t;
typedef struct pod pod_t;
typedef struct deployment deployment_t;
typedef struct service service_t;

// Orchestrator operations
orchestrator_t* orchestrator_create(const char* cluster_name);
void orchestrator_destroy(orchestrator_t* orch);

// Pod operations (group of containers)
pod_t* pod_create(const char* name);
void pod_destroy(pod_t* pod);
int pod_add_container(pod_t* pod, container_t* container);
int pod_start(pod_t* pod);
int pod_stop(pod_t* pod);
int orchestrator_deploy_pod(orchestrator_t* orch, pod_t* pod);
int orchestrator_delete_pod(orchestrator_t* orch, const char* pod_name);

// Deployment operations (replica management)
deployment_t* deployment_create(const char* name, int replicas);
void deployment_destroy(deployment_t* deployment);
int deployment_set_template(deployment_t* deployment, const container_config_t* config);
int deployment_scale(deployment_t* deployment, int replicas);
int orchestrator_apply_deployment(orchestrator_t* orch, deployment_t* deployment);
int orchestrator_delete_deployment(orchestrator_t* orch, const char* deployment_name);

// Service operations (load balancing)
service_t* service_create(const char* name, const char* selector);
void service_destroy(service_t* service);
int service_add_port(service_t* service, int port, int target_port, 
                    const char* protocol);
int orchestrator_create_service(orchestrator_t* orch, service_t* service);
int orchestrator_delete_service(orchestrator_t* orch, const char* service_name);

// ============================================================================
// Container Registry
// ============================================================================

typedef struct container_registry container_registry_t;

container_registry_t* container_registry_create(const char* url);
void container_registry_destroy(container_registry_t* registry);
int container_registry_login(container_registry_t* registry, const char* username,
                            const char* password);
int container_registry_push_image(container_registry_t* registry, 
                                 const char* image_name);
int container_registry_pull_image(container_registry_t* registry,
                                 const char* image_name);
char** container_registry_list_images(container_registry_t* registry, size_t* count);

#endif // CONTAINER_H
