#ifndef CRYPTO_H
#define CRYPTO_H

#include "common.h"

// ============================================================================
// Hashing Algorithms
// ============================================================================

typedef enum {
    HASH_MD5,
    HASH_SHA1,
    HASH_SHA224,
    HASH_SHA256,
    HASH_SHA384,
    HASH_SHA512,
    HASH_SHA3_256,
    HASH_SHA3_512
} hash_algorithm_t;

// Hash operations
int crypto_hash(hash_algorithm_t algorithm, const void* data, size_t data_len,
               void* output, size_t* output_len);
char* crypto_hash_hex(hash_algorithm_t algorithm, const void* data, size_t data_len);
int crypto_hash_file(hash_algorithm_t algorithm, const char* filename,
                     void* output, size_t* output_len);

// ============================================================================
// Password Hashing with Salt
// ============================================================================

typedef enum {
    PASSWORD_HASH_PBKDF2,
    PASSWORD_HASH_BCRYPT,
    PASSWORD_HASH_SCRYPT,
    PASSWORD_HASH_ARGON2
} password_hash_algorithm_t;

typedef struct password_hash password_hash_t;

// Password hashing operations
password_hash_t* password_hash_create(const char* password, 
                                     password_hash_algorithm_t algorithm);
void password_hash_destroy(password_hash_t* hash);
char* password_hash_serialize(const password_hash_t* hash);
password_hash_t* password_hash_deserialize(const char* hash_str);
int password_hash_verify(const password_hash_t* hash, const char* password);
int password_hash_needs_rehash(const password_hash_t* hash, 
                               password_hash_algorithm_t algorithm);

// Salt generation
int crypto_generate_salt(void* salt, size_t salt_len);
char* crypto_generate_salt_hex(size_t salt_len);

// ============================================================================
// Symmetric Encryption (AES)
// ============================================================================

typedef enum {
    AES_128_ECB,
    AES_128_CBC,
    AES_128_CTR,
    AES_128_GCM,
    AES_256_ECB,
    AES_256_CBC,
    AES_256_CTR,
    AES_256_GCM
} aes_mode_t;

typedef struct aes_context aes_context_t;

// AES operations
aes_context_t* aes_context_create(aes_mode_t mode, const void* key, size_t key_len);
void aes_context_destroy(aes_context_t* ctx);
int aes_encrypt(aes_context_t* ctx, const void* plaintext, size_t plaintext_len,
               void* ciphertext, size_t* ciphertext_len, const void* iv);
int aes_decrypt(aes_context_t* ctx, const void* ciphertext, size_t ciphertext_len,
               void* plaintext, size_t* plaintext_len, const void* iv);

// ============================================================================
// Asymmetric Encryption (RSA)
// ============================================================================

typedef enum {
    RSA_1024,
    RSA_2048,
    RSA_4096
} rsa_key_size_t;

typedef struct rsa_keypair rsa_keypair_t;
typedef struct rsa_public_key rsa_public_key_t;
typedef struct rsa_private_key rsa_private_key_t;

// RSA key operations
rsa_keypair_t* rsa_keypair_generate(rsa_key_size_t key_size);
void rsa_keypair_destroy(rsa_keypair_t* keypair);
rsa_public_key_t* rsa_keypair_get_public(const rsa_keypair_t* keypair);
rsa_private_key_t* rsa_keypair_get_private(const rsa_keypair_t* keypair);
char* rsa_public_key_export_pem(const rsa_public_key_t* key);
char* rsa_private_key_export_pem(const rsa_private_key_t* key);
rsa_public_key_t* rsa_public_key_import_pem(const char* pem);
rsa_private_key_t* rsa_private_key_import_pem(const char* pem);
void rsa_public_key_destroy(rsa_public_key_t* key);
void rsa_private_key_destroy(rsa_private_key_t* key);

// RSA encryption/decryption
int rsa_encrypt(const rsa_public_key_t* key, const void* plaintext, 
               size_t plaintext_len, void* ciphertext, size_t* ciphertext_len);
int rsa_decrypt(const rsa_private_key_t* key, const void* ciphertext,
               size_t ciphertext_len, void* plaintext, size_t* plaintext_len);

// ============================================================================
// Digital Signatures
// ============================================================================

typedef enum {
    SIGNATURE_RSA_SHA256,
    SIGNATURE_RSA_SHA512,
    SIGNATURE_ECDSA_SHA256,
    SIGNATURE_ECDSA_SHA512
} signature_algorithm_t;

// Signature operations
int crypto_sign(signature_algorithm_t algorithm, const rsa_private_key_t* key,
               const void* data, size_t data_len, void* signature, size_t* sig_len);
int crypto_verify(signature_algorithm_t algorithm, const rsa_public_key_t* key,
                 const void* data, size_t data_len, const void* signature, 
                 size_t sig_len);

// ============================================================================
// Random Number Generation
// ============================================================================

int crypto_random_bytes(void* buffer, size_t length);
uint32_t crypto_random_uint32(void);
uint64_t crypto_random_uint64(void);
int crypto_random_range(int min, int max);

// ============================================================================
// Key Derivation Functions
// ============================================================================

int crypto_kdf_pbkdf2(const void* password, size_t password_len,
                     const void* salt, size_t salt_len,
                     int iterations, hash_algorithm_t hash_alg,
                     void* output, size_t output_len);

// ============================================================================
// Message Authentication Code (HMAC)
// ============================================================================

int crypto_hmac(hash_algorithm_t algorithm, const void* key, size_t key_len,
               const void* data, size_t data_len, void* output, size_t* output_len);
char* crypto_hmac_hex(hash_algorithm_t algorithm, const void* key, size_t key_len,
                     const void* data, size_t data_len);

#endif // CRYPTO_H
