#ifndef SECURITY_H
#define SECURITY_H

#include "common.h"

// ============================================================================
// Rate Limiting
// ============================================================================

typedef enum {
    RATE_LIMIT_FIXED_WINDOW,
    RATE_LIMIT_SLIDING_WINDOW,
    RATE_LIMIT_TOKEN_BUCKET,
    RATE_LIMIT_LEAKY_BUCKET
} rate_limit_algorithm_t;

typedef struct rate_limiter_advanced rate_limiter_advanced_t;

// Advanced rate limiter operations
rate_limiter_advanced_t* rate_limiter_advanced_create(rate_limit_algorithm_t algorithm,
                                                      int max_requests,
                                                      uint64_t window_ms);
void rate_limiter_advanced_destroy(rate_limiter_advanced_t* limiter);
int rate_limiter_advanced_allow(rate_limiter_advanced_t* limiter, const char* client_id);
int rate_limiter_advanced_try_acquire(rate_limiter_advanced_t* limiter, 
                                     const char* client_id, int tokens);
int rate_limiter_advanced_get_remaining(rate_limiter_advanced_t* limiter, 
                                       const char* client_id);
uint64_t rate_limiter_advanced_get_reset_time(rate_limiter_advanced_t* limiter,
                                              const char* client_id);
int rate_limiter_advanced_reset(rate_limiter_advanced_t* limiter, const char* client_id);

// ============================================================================
// DDoS Protection
// ============================================================================

typedef struct ddos_protector ddos_protector_t;

typedef enum {
    DDOS_ACTION_ALLOW,
    DDOS_ACTION_CHALLENGE,
    DDOS_ACTION_RATE_LIMIT,
    DDOS_ACTION_BLOCK
} ddos_action_t;

typedef struct {
    int syn_flood_threshold;
    int connection_rate_threshold;
    int request_rate_threshold;
    uint64_t window_ms;
    int max_connections_per_ip;
    int enable_geo_blocking;
    int enable_challenge_response;
} ddos_config_t;

// DDoS protection operations
ddos_protector_t* ddos_protector_create(const ddos_config_t* config);
void ddos_protector_destroy(ddos_protector_t* protector);
ddos_action_t ddos_protector_check_request(ddos_protector_t* protector, 
                                          const char* client_ip,
                                          const char* user_agent);
int ddos_protector_record_connection(ddos_protector_t* protector, const char* client_ip);
int ddos_protector_record_request(ddos_protector_t* protector, const char* client_ip);
int ddos_protector_blacklist_ip(ddos_protector_t* protector, const char* ip, 
                                uint64_t duration_ms);
int ddos_protector_whitelist_ip(ddos_protector_t* protector, const char* ip);
int ddos_protector_is_blacklisted(ddos_protector_t* protector, const char* ip);

// ============================================================================
// Input Validation & Sanitization
// ============================================================================

// SQL Injection prevention
int security_validate_sql_safe(const char* input);
char* security_escape_sql(const char* input);

// XSS (Cross-Site Scripting) prevention
char* security_escape_html(const char* input);
char* security_escape_javascript(const char* input);
int security_validate_html_safe(const char* input);

// Path traversal prevention
int security_validate_path(const char* path);
char* security_normalize_path(const char* path);

// General input validation
int security_validate_email(const char* email);
int security_validate_url(const char* url);
int security_validate_ipv4(const char* ip);
int security_validate_ipv6(const char* ip);
int security_validate_domain(const char* domain);

// ============================================================================
// CORS (Cross-Origin Resource Sharing)
// ============================================================================

typedef struct cors_config cors_config_t;

struct cors_config {
    char** allowed_origins;
    size_t allowed_origins_count;
    char** allowed_methods;
    size_t allowed_methods_count;
    char** allowed_headers;
    size_t allowed_headers_count;
    char** exposed_headers;
    size_t exposed_headers_count;
    int allow_credentials;
    uint64_t max_age;
};

cors_config_t* cors_config_create(void);
void cors_config_destroy(cors_config_t* config);
int cors_config_add_origin(cors_config_t* config, const char* origin);
int cors_config_add_method(cors_config_t* config, const char* method);
int cors_config_add_header(cors_config_t* config, const char* header);
int cors_is_allowed(const cors_config_t* config, const char* origin, const char* method);
char* cors_get_headers(const cors_config_t* config, const char* origin);

// ============================================================================
// CSRF (Cross-Site Request Forgery) Protection
// ============================================================================

typedef struct csrf_protector csrf_protector_t;

csrf_protector_t* csrf_protector_create(const char* secret_key);
void csrf_protector_destroy(csrf_protector_t* protector);
char* csrf_generate_token(csrf_protector_t* protector, const char* session_id);
int csrf_validate_token(csrf_protector_t* protector, const char* token, 
                       const char* session_id);

// ============================================================================
// Content Security Policy (CSP)
// ============================================================================

typedef struct csp_policy csp_policy_t;

csp_policy_t* csp_policy_create(void);
void csp_policy_destroy(csp_policy_t* policy);
int csp_policy_add_directive(csp_policy_t* policy, const char* directive, 
                             const char* value);
char* csp_policy_serialize(const csp_policy_t* policy);

// ============================================================================
// Security Headers
// ============================================================================

typedef struct security_headers security_headers_t;

struct security_headers {
    int enable_hsts;                    // HTTP Strict Transport Security
    int hsts_max_age;
    int hsts_include_subdomains;
    int enable_x_frame_options;         // Clickjacking protection
    char* x_frame_options_value;
    int enable_x_content_type_options;  // MIME sniffing protection
    int enable_x_xss_protection;        // XSS filter
    int enable_referrer_policy;
    char* referrer_policy_value;
};

security_headers_t* security_headers_create_default(void);
void security_headers_destroy(security_headers_t* headers);
char* security_headers_serialize(const security_headers_t* headers);

// ============================================================================
// API Key Management
// ============================================================================

typedef struct api_key_manager api_key_manager_t;
typedef struct api_key api_key_t;

api_key_manager_t* api_key_manager_create(void);
void api_key_manager_destroy(api_key_manager_t* manager);
api_key_t* api_key_generate(api_key_manager_t* manager, const char* name, 
                           const char** scopes, size_t scope_count);
int api_key_validate(api_key_manager_t* manager, const char* key);
int api_key_revoke(api_key_manager_t* manager, const char* key);
int api_key_has_scope(const api_key_t* key, const char* scope);
void api_key_destroy(api_key_t* key);

#endif // SECURITY_H
