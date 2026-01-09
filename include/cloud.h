#ifndef CLOUD_H
#define CLOUD_H

#include "common.h"

// ============================================================================
// Cloud Provider Abstraction
// ============================================================================

typedef enum {
    CLOUD_PROVIDER_AWS,
    CLOUD_PROVIDER_GCP,
    CLOUD_PROVIDER_AZURE
} cloud_provider_t;

typedef struct cloud_client cloud_client_t;

cloud_client_t* cloud_client_create(cloud_provider_t provider, const char* credentials);
void cloud_client_destroy(cloud_client_t* client);

// ============================================================================
// Compute Services (EC2, Compute Engine, Virtual Machines)
// ============================================================================

typedef struct cloud_vm cloud_vm_t;
typedef struct cloud_vm_config cloud_vm_config_t;

struct cloud_vm_config {
    const char* instance_type;
    const char* image_id;
    const char* region;
    const char* availability_zone;
    int min_count;
    int max_count;
    const char* key_pair_name;
    const char** security_groups;
    size_t security_group_count;
};

cloud_vm_t* cloud_vm_create(cloud_client_t* client, const cloud_vm_config_t* config);
void cloud_vm_destroy(cloud_vm_t* vm);
int cloud_vm_start(cloud_vm_t* vm);
int cloud_vm_stop(cloud_vm_t* vm);
int cloud_vm_terminate(cloud_vm_t* vm);
int cloud_vm_reboot(cloud_vm_t* vm);
const char* cloud_vm_get_id(const cloud_vm_t* vm);
const char* cloud_vm_get_public_ip(const cloud_vm_t* vm);
const char* cloud_vm_get_private_ip(const cloud_vm_t* vm);

// ============================================================================
// Object Storage (S3, Cloud Storage, Blob Storage)
// ============================================================================

typedef struct cloud_storage cloud_storage_t;
typedef struct cloud_bucket cloud_bucket_t;
typedef struct cloud_object cloud_object_t;

cloud_storage_t* cloud_storage_create(cloud_client_t* client);
void cloud_storage_destroy(cloud_storage_t* storage);

// Bucket operations
cloud_bucket_t* cloud_storage_create_bucket(cloud_storage_t* storage, 
                                           const char* bucket_name,
                                           const char* region);
int cloud_storage_delete_bucket(cloud_storage_t* storage, const char* bucket_name);
cloud_bucket_t* cloud_storage_get_bucket(cloud_storage_t* storage, 
                                        const char* bucket_name);
char** cloud_storage_list_buckets(cloud_storage_t* storage, size_t* count);

// Object operations
int cloud_bucket_upload_object(cloud_bucket_t* bucket, const char* key,
                              const void* data, size_t data_len);
int cloud_bucket_upload_file(cloud_bucket_t* bucket, const char* key,
                            const char* file_path);
int cloud_bucket_download_object(cloud_bucket_t* bucket, const char* key,
                                void** data, size_t* data_len);
int cloud_bucket_download_file(cloud_bucket_t* bucket, const char* key,
                              const char* file_path);
int cloud_bucket_delete_object(cloud_bucket_t* bucket, const char* key);
char** cloud_bucket_list_objects(cloud_bucket_t* bucket, const char* prefix, 
                                size_t* count);
char* cloud_bucket_generate_presigned_url(cloud_bucket_t* bucket, const char* key,
                                         uint64_t expiration_seconds);

void cloud_bucket_destroy(cloud_bucket_t* bucket);

// ============================================================================
// Database Services (RDS, Cloud SQL, Azure Database)
// ============================================================================

typedef struct cloud_database cloud_database_t;
typedef struct cloud_db_config cloud_db_config_t;

typedef enum {
    CLOUD_DB_ENGINE_MYSQL,
    CLOUD_DB_ENGINE_POSTGRESQL,
    CLOUD_DB_ENGINE_MSSQL,
    CLOUD_DB_ENGINE_ORACLE
} cloud_db_engine_t;

struct cloud_db_config {
    cloud_db_engine_t engine;
    const char* engine_version;
    const char* instance_class;
    int allocated_storage_gb;
    const char* db_name;
    const char* master_username;
    const char* master_password;
    int multi_az;
    int publicly_accessible;
    const char** security_groups;
    size_t security_group_count;
};

cloud_database_t* cloud_database_create(cloud_client_t* client,
                                        const cloud_db_config_t* config);
void cloud_database_destroy(cloud_database_t* db);
int cloud_database_start(cloud_database_t* db);
int cloud_database_stop(cloud_database_t* db);
int cloud_database_delete(cloud_database_t* db);
const char* cloud_database_get_endpoint(const cloud_database_t* db);
int cloud_database_get_port(const cloud_database_t* db);
int cloud_database_create_snapshot(cloud_database_t* db, const char* snapshot_id);

// ============================================================================
// NoSQL Services (DynamoDB, Firestore, Cosmos DB)
// ============================================================================

typedef struct cloud_nosql cloud_nosql_t;
typedef struct cloud_nosql_table cloud_nosql_table_t;

cloud_nosql_t* cloud_nosql_create(cloud_client_t* client);
void cloud_nosql_destroy(cloud_nosql_t* nosql);
cloud_nosql_table_t* cloud_nosql_create_table(cloud_nosql_t* nosql,
                                              const char* table_name,
                                              const char* partition_key,
                                              const char* sort_key);
int cloud_nosql_delete_table(cloud_nosql_t* nosql, const char* table_name);
int cloud_nosql_put_item(cloud_nosql_table_t* table, const char* item_json);
char* cloud_nosql_get_item(cloud_nosql_table_t* table, const char* key);
int cloud_nosql_delete_item(cloud_nosql_table_t* table, const char* key);
char** cloud_nosql_query(cloud_nosql_table_t* table, const char* query_expression,
                        size_t* count);

// ============================================================================
// Message Queue Services (SQS, Pub/Sub, Service Bus)
// ============================================================================

typedef struct cloud_queue cloud_queue_t;

cloud_queue_t* cloud_queue_create(cloud_client_t* client, const char* queue_name);
void cloud_queue_destroy(cloud_queue_t* queue);
int cloud_queue_send_message(cloud_queue_t* queue, const char* message);
char* cloud_queue_receive_message(cloud_queue_t* queue, uint64_t wait_time_seconds);
int cloud_queue_delete_message(cloud_queue_t* queue, const char* receipt_handle);
int cloud_queue_purge(cloud_queue_t* queue);

// ============================================================================
// Lambda/Cloud Functions (Serverless)
// ============================================================================

typedef struct cloud_function cloud_function_t;
typedef struct cloud_function_config cloud_function_config_t;

struct cloud_function_config {
    const char* function_name;
    const char* runtime;
    const char* handler;
    const char* code_zip_path;
    int memory_size_mb;
    int timeout_seconds;
    const char** env_vars;
    size_t env_var_count;
    const char* role_arn;
};

cloud_function_t* cloud_function_create(cloud_client_t* client,
                                       const cloud_function_config_t* config);
void cloud_function_destroy(cloud_function_t* function);
char* cloud_function_invoke(cloud_function_t* function, const char* payload);
int cloud_function_update_code(cloud_function_t* function, const char* code_zip_path);
int cloud_function_delete(cloud_function_t* function);

// ============================================================================
// CDN (CloudFront, Cloud CDN, Azure CDN)
// ============================================================================

typedef struct cloud_cdn cloud_cdn_t;
typedef struct cloud_cdn_distribution cloud_cdn_distribution_t;

cloud_cdn_t* cloud_cdn_create(cloud_client_t* client);
void cloud_cdn_destroy(cloud_cdn_t* cdn);
cloud_cdn_distribution_t* cloud_cdn_create_distribution(cloud_cdn_t* cdn,
                                                        const char* origin_domain);
int cloud_cdn_delete_distribution(cloud_cdn_t* cdn, const char* distribution_id);
int cloud_cdn_invalidate_cache(cloud_cdn_distribution_t* dist, 
                              const char** paths, size_t path_count);
const char* cloud_cdn_distribution_get_domain(const cloud_cdn_distribution_t* dist);

// ============================================================================
// Load Balancer (ELB, Cloud Load Balancing, Load Balancer)
// ============================================================================

typedef struct cloud_load_balancer cloud_load_balancer_t;

cloud_load_balancer_t* cloud_load_balancer_create(cloud_client_t* client,
                                                  const char* name,
                                                  const char** availability_zones,
                                                  size_t zone_count);
void cloud_load_balancer_destroy(cloud_load_balancer_t* lb);
int cloud_load_balancer_register_instances(cloud_load_balancer_t* lb,
                                          const char** instance_ids,
                                          size_t instance_count);
int cloud_load_balancer_deregister_instances(cloud_load_balancer_t* lb,
                                            const char** instance_ids,
                                            size_t instance_count);
const char* cloud_load_balancer_get_dns(const cloud_load_balancer_t* lb);

// ============================================================================
// DNS Service (Route 53, Cloud DNS, Azure DNS)
// ============================================================================

typedef struct cloud_dns cloud_dns_t;
typedef struct cloud_dns_zone cloud_dns_zone_t;

cloud_dns_t* cloud_dns_create(cloud_client_t* client);
void cloud_dns_destroy(cloud_dns_t* dns);
cloud_dns_zone_t* cloud_dns_create_zone(cloud_dns_t* dns, const char* domain_name);
int cloud_dns_delete_zone(cloud_dns_t* dns, const char* zone_id);
int cloud_dns_add_record(cloud_dns_zone_t* zone, const char* name, 
                        const char* type, const char* value, int ttl);
int cloud_dns_delete_record(cloud_dns_zone_t* zone, const char* name, 
                           const char* type);

// ============================================================================
// Identity and Access Management (IAM)
// ============================================================================

typedef struct cloud_iam cloud_iam_t;
typedef struct cloud_user cloud_user_t;
typedef struct cloud_role cloud_role_t;

cloud_iam_t* cloud_iam_create(cloud_client_t* client);
void cloud_iam_destroy(cloud_iam_t* iam);
cloud_user_t* cloud_iam_create_user(cloud_iam_t* iam, const char* username);
int cloud_iam_delete_user(cloud_iam_t* iam, const char* username);
int cloud_iam_attach_policy(cloud_iam_t* iam, const char* user_or_role,
                           const char* policy_arn);
cloud_role_t* cloud_iam_create_role(cloud_iam_t* iam, const char* role_name,
                                    const char* trust_policy);
int cloud_iam_delete_role(cloud_iam_t* iam, const char* role_name);

// ============================================================================
// Monitoring and Logging (CloudWatch, Stackdriver, Azure Monitor)
// ============================================================================

typedef struct cloud_monitoring cloud_monitoring_t;

cloud_monitoring_t* cloud_monitoring_create(cloud_client_t* client);
void cloud_monitoring_destroy(cloud_monitoring_t* monitoring);
int cloud_monitoring_put_metric(cloud_monitoring_t* monitoring, 
                               const char* namespace, const char* metric_name,
                               double value);
int cloud_monitoring_create_alarm(cloud_monitoring_t* monitoring,
                                 const char* alarm_name,
                                 const char* metric_name,
                                 const char* comparison,
                                 double threshold);
char** cloud_monitoring_get_logs(cloud_monitoring_t* monitoring,
                                const char* log_group,
                                uint64_t start_time, uint64_t end_time,
                                size_t* count);

#endif // CLOUD_H
