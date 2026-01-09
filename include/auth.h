#ifndef AUTH_H
#define AUTH_H

#include "common.h"

// ============================================================================
// JWT (JSON Web Token) Implementation
// ============================================================================

typedef enum {
    JWT_ALG_HS256,  // HMAC with SHA-256
    JWT_ALG_HS384,  // HMAC with SHA-384
    JWT_ALG_HS512,  // HMAC with SHA-512
    JWT_ALG_RS256,  // RSA with SHA-256
    JWT_ALG_RS384,  // RSA with SHA-384
    JWT_ALG_RS512   // RSA with SHA-512
} jwt_algorithm_t;

typedef struct jwt_token jwt_token_t;

// JWT operations
jwt_token_t* jwt_create(jwt_algorithm_t algorithm, const char* secret);
void jwt_destroy(jwt_token_t* token);
int jwt_add_claim(jwt_token_t* token, const char* key, const char* value);
int jwt_add_claim_int(jwt_token_t* token, const char* key, int64_t value);
int jwt_set_expiration(jwt_token_t* token, uint64_t exp_timestamp);
int jwt_set_issued_at(jwt_token_t* token, uint64_t iat_timestamp);
int jwt_set_not_before(jwt_token_t* token, uint64_t nbf_timestamp);
char* jwt_encode(const jwt_token_t* token);
jwt_token_t* jwt_decode(const char* token_str, const char* secret);
int jwt_verify(const jwt_token_t* token, const char* secret);
const char* jwt_get_claim(const jwt_token_t* token, const char* key);
int64_t jwt_get_claim_int(const jwt_token_t* token, const char* key);

// ============================================================================
// Session Management
// ============================================================================

typedef struct session_manager session_manager_t;
typedef struct session session_t;

typedef enum {
    SESSION_STORAGE_MEMORY,
    SESSION_STORAGE_REDIS,
    SESSION_STORAGE_DATABASE
} session_storage_t;

// Session operations
session_manager_t* session_manager_create(session_storage_t storage, uint64_t default_ttl_seconds);
void session_manager_destroy(session_manager_t* manager);
session_t* session_create(session_manager_t* manager, const char* user_id);
void session_destroy(session_t* session);
int session_set_data(session_t* session, const char* key, const void* value, size_t value_len);
int session_get_data(session_t* session, const char* key, void** value, size_t* value_len);
int session_remove_data(session_t* session, const char* key);
int session_validate(session_manager_t* manager, const char* session_id);
int session_invalidate(session_manager_t* manager, const char* session_id);
int session_refresh(session_manager_t* manager, const char* session_id);
const char* session_get_id(const session_t* session);
const char* session_get_user_id(const session_t* session);

// ============================================================================
// OAuth 2.0 Implementation
// ============================================================================

typedef enum {
    OAUTH_GRANT_AUTHORIZATION_CODE,
    OAUTH_GRANT_IMPLICIT,
    OAUTH_GRANT_PASSWORD,
    OAUTH_GRANT_CLIENT_CREDENTIALS,
    OAUTH_GRANT_REFRESH_TOKEN
} oauth_grant_type_t;

typedef struct oauth_server oauth_server_t;
typedef struct oauth_client oauth_client_t;
typedef struct oauth_token oauth_token_t;

// OAuth Server operations
oauth_server_t* oauth_server_create(const char* issuer_url);
void oauth_server_destroy(oauth_server_t* server);
int oauth_server_register_client(oauth_server_t* server, const char* client_id, 
                                  const char* client_secret, const char* redirect_uri);
char* oauth_server_generate_authorization_code(oauth_server_t* server, 
                                               const char* client_id, const char* user_id,
                                               const char* scope);
oauth_token_t* oauth_server_exchange_code(oauth_server_t* server, 
                                          const char* code, const char* client_id);
oauth_token_t* oauth_server_refresh_token(oauth_server_t* server, 
                                          const char* refresh_token);
int oauth_server_validate_token(oauth_server_t* server, const char* access_token);

// OAuth Client operations
oauth_client_t* oauth_client_create(const char* client_id, const char* client_secret,
                                    const char* authorization_endpoint,
                                    const char* token_endpoint);
void oauth_client_destroy(oauth_client_t* client);
char* oauth_client_get_authorization_url(oauth_client_t* client, 
                                         const char* redirect_uri, const char* scope);
oauth_token_t* oauth_client_exchange_code(oauth_client_t* client, 
                                         const char* code, const char* redirect_uri);
oauth_token_t* oauth_client_refresh_token(oauth_client_t* client, 
                                         const char* refresh_token);

// OAuth Token operations
void oauth_token_destroy(oauth_token_t* token);
const char* oauth_token_get_access_token(const oauth_token_t* token);
const char* oauth_token_get_refresh_token(const oauth_token_t* token);
const char* oauth_token_get_token_type(const oauth_token_t* token);
uint64_t oauth_token_get_expires_in(const oauth_token_t* token);
const char* oauth_token_get_scope(const oauth_token_t* token);

// ============================================================================
// OpenID Connect (OIDC) Implementation
// ============================================================================

typedef struct oidc_provider oidc_provider_t;
typedef struct oidc_id_token oidc_id_token_t;

// OIDC Provider operations
oidc_provider_t* oidc_provider_create(const char* issuer, const char* discovery_url);
void oidc_provider_destroy(oidc_provider_t* provider);
int oidc_provider_discover(oidc_provider_t* provider);
char* oidc_provider_get_authorization_url(oidc_provider_t* provider, 
                                         const char* client_id,
                                         const char* redirect_uri, const char* scope);
oidc_id_token_t* oidc_provider_exchange_code(oidc_provider_t* provider,
                                             const char* code, const char* client_id,
                                             const char* client_secret, 
                                             const char* redirect_uri);
int oidc_provider_validate_id_token(oidc_provider_t* provider, const char* id_token);

// OIDC ID Token operations
void oidc_id_token_destroy(oidc_id_token_t* token);
const char* oidc_id_token_get_subject(const oidc_id_token_t* token);
const char* oidc_id_token_get_email(const oidc_id_token_t* token);
const char* oidc_id_token_get_name(const oidc_id_token_t* token);
const char* oidc_id_token_get_claim(const oidc_id_token_t* token, const char* key);
uint64_t oidc_id_token_get_expiration(const oidc_id_token_t* token);
uint64_t oidc_id_token_get_issued_at(const oidc_id_token_t* token);

#endif // AUTH_H
