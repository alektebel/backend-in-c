#ifndef CICD_H
#define CICD_H

#include "common.h"

// ============================================================================
// CI/CD Pipeline
// ============================================================================

typedef struct pipeline pipeline_t;
typedef struct pipeline_stage pipeline_stage_t;
typedef struct pipeline_job pipeline_job_t;
typedef struct pipeline_run pipeline_run_t;

typedef enum {
    JOB_STATUS_PENDING,
    JOB_STATUS_RUNNING,
    JOB_STATUS_SUCCESS,
    JOB_STATUS_FAILED,
    JOB_STATUS_SKIPPED,
    JOB_STATUS_CANCELLED
} job_status_t;

// Pipeline operations
pipeline_t* pipeline_create(const char* name);
void pipeline_destroy(pipeline_t* pipeline);
int pipeline_add_stage(pipeline_t* pipeline, pipeline_stage_t* stage);
int pipeline_load_from_file(pipeline_t* pipeline, const char* config_file);
pipeline_run_t* pipeline_execute(pipeline_t* pipeline);

// Stage operations
pipeline_stage_t* pipeline_stage_create(const char* name);
void pipeline_stage_destroy(pipeline_stage_t* stage);
int pipeline_stage_add_job(pipeline_stage_t* stage, pipeline_job_t* job);
int pipeline_stage_set_condition(pipeline_stage_t* stage, const char* condition);

// Job operations
pipeline_job_t* pipeline_job_create(const char* name);
void pipeline_job_destroy(pipeline_job_t* job);
int pipeline_job_add_command(pipeline_job_t* job, const char* command);
int pipeline_job_set_docker_image(pipeline_job_t* job, const char* image);
int pipeline_job_add_env_var(pipeline_job_t* job, const char* key, const char* value);
int pipeline_job_set_timeout(pipeline_job_t* job, uint64_t timeout_seconds);
int pipeline_job_set_retry_count(pipeline_job_t* job, int retries);

// Pipeline run operations
void pipeline_run_destroy(pipeline_run_t* run);
job_status_t pipeline_run_get_status(const pipeline_run_t* run);
const char* pipeline_run_get_logs(const pipeline_run_t* run);
uint64_t pipeline_run_get_duration(const pipeline_run_t* run);

// ============================================================================
// GitHub Actions
// ============================================================================

typedef struct github_workflow github_workflow_t;
typedef struct github_action github_action_t;

github_workflow_t* github_workflow_create(const char* name);
void github_workflow_destroy(github_workflow_t* workflow);
int github_workflow_load_yaml(github_workflow_t* workflow, const char* yaml_file);
int github_workflow_set_trigger(github_workflow_t* workflow, const char* event);
int github_workflow_add_job(github_workflow_t* workflow, const char* job_id,
                           pipeline_job_t* job);
int github_workflow_execute(github_workflow_t* workflow);

// ============================================================================
// GitLab CI
// ============================================================================

typedef struct gitlab_pipeline gitlab_pipeline_t;

gitlab_pipeline_t* gitlab_pipeline_create(const char* name);
void gitlab_pipeline_destroy(gitlab_pipeline_t* pipeline);
int gitlab_pipeline_load_yaml(gitlab_pipeline_t* pipeline, const char* yaml_file);
int gitlab_pipeline_add_stage(gitlab_pipeline_t* pipeline, const char* stage_name);
int gitlab_pipeline_add_job(gitlab_pipeline_t* pipeline, const char* job_name,
                           const char* stage, const char* script);
int gitlab_pipeline_execute(gitlab_pipeline_t* pipeline);

// ============================================================================
// Artifact Management
// ============================================================================

typedef struct artifact_manager artifact_manager_t;
typedef struct artifact artifact_t;

artifact_manager_t* artifact_manager_create(const char* storage_path);
void artifact_manager_destroy(artifact_manager_t* manager);
int artifact_manager_upload(artifact_manager_t* manager, const char* name,
                           const char* file_path);
int artifact_manager_download(artifact_manager_t* manager, const char* name,
                             const char* destination_path);
char** artifact_manager_list(artifact_manager_t* manager, size_t* count);
int artifact_manager_delete(artifact_manager_t* manager, const char* name);

// ============================================================================
// Build System Integration
// ============================================================================

typedef struct build_system build_system_t;

typedef enum {
    BUILD_SYSTEM_MAKE,
    BUILD_SYSTEM_CMAKE,
    BUILD_SYSTEM_GRADLE,
    BUILD_SYSTEM_MAVEN,
    BUILD_SYSTEM_NPM
} build_system_type_t;

build_system_t* build_system_create(build_system_type_t type);
void build_system_destroy(build_system_t* system);
int build_system_configure(build_system_t* system, const char* config_file);
int build_system_build(build_system_t* system);
int build_system_test(build_system_t* system);
int build_system_package(build_system_t* system);

// ============================================================================
// Deployment Strategies
// ============================================================================

typedef struct deployment deployment_strategy_t;

typedef enum {
    DEPLOY_STRATEGY_BLUE_GREEN,
    DEPLOY_STRATEGY_CANARY,
    DEPLOY_STRATEGY_ROLLING,
    DEPLOY_STRATEGY_RECREATE
} deployment_strategy_type_t;

deployment_strategy_t* deployment_strategy_create(deployment_strategy_type_t type);
void deployment_strategy_destroy(deployment_strategy_t* strategy);
int deployment_strategy_set_target(deployment_strategy_t* strategy, 
                                  const char* target_url);
int deployment_strategy_deploy(deployment_strategy_t* strategy, 
                              const char* artifact_path);
int deployment_strategy_rollback(deployment_strategy_t* strategy);

// ============================================================================
// Secret Management
// ============================================================================

typedef struct secret_manager secret_manager_t;

secret_manager_t* secret_manager_create(const char* vault_url);
void secret_manager_destroy(secret_manager_t* manager);
int secret_manager_store(secret_manager_t* manager, const char* key, 
                        const char* value);
char* secret_manager_retrieve(secret_manager_t* manager, const char* key);
int secret_manager_delete(secret_manager_t* manager, const char* key);
int secret_manager_inject_into_env(secret_manager_t* manager, const char* key);

// ============================================================================
// Environment Management
// ============================================================================

typedef struct environment environment_t;

typedef enum {
    ENV_TYPE_DEVELOPMENT,
    ENV_TYPE_STAGING,
    ENV_TYPE_PRODUCTION
} environment_type_t;

environment_t* environment_create(const char* name, environment_type_t type);
void environment_destroy(environment_t* env);
int environment_set_variable(environment_t* env, const char* key, const char* value);
const char* environment_get_variable(environment_t* env, const char* key);
int environment_deploy(environment_t* env, const char* artifact_path);

// ============================================================================
// Notification System
// ============================================================================

typedef struct notification_manager notification_manager_t;

typedef enum {
    NOTIFICATION_EMAIL,
    NOTIFICATION_SLACK,
    NOTIFICATION_WEBHOOK
} notification_type_t;

notification_manager_t* notification_manager_create(void);
void notification_manager_destroy(notification_manager_t* manager);
int notification_manager_add_channel(notification_manager_t* manager,
                                    notification_type_t type, const char* config);
int notification_manager_send_build_status(notification_manager_t* manager,
                                          const char* status, const char* details);
int notification_manager_send_deployment_status(notification_manager_t* manager,
                                               const char* environment,
                                               const char* status);

#endif // CICD_H
