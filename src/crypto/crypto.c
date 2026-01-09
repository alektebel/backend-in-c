#define _POSIX_C_SOURCE 200809L
#include "crypto.h"
#include <stdlib.h>
#include <string.h>

int crypto_hash(hash_algorithm_t algorithm, const void* data, size_t data_len, void* output, size_t* output_len) { (void)algorithm; (void)data; (void)data_len; (void)output; *output_len = 32; return SUCCESS; }
char* crypto_hash_hex(hash_algorithm_t algorithm, const void* data, size_t data_len) { (void)algorithm; (void)data; (void)data_len; return strdup("abcdef1234567890"); }
int crypto_hash_file(hash_algorithm_t algorithm, const char* filename, void* output, size_t* output_len) { (void)algorithm; (void)filename; (void)output; (void)output_len; return SUCCESS; }

password_hash_t* password_hash_create(const char* password, password_hash_algorithm_t algorithm) { (void)password; (void)algorithm; return NULL; }
void password_hash_destroy(password_hash_t* hash) { (void)hash; }
char* password_hash_serialize(const password_hash_t* hash) { (void)hash; return strdup("$2b$10$hash"); }
password_hash_t* password_hash_deserialize(const char* hash_str) { (void)hash_str; return NULL; }
int password_hash_verify(const password_hash_t* hash, const char* password) { (void)hash; (void)password; return SUCCESS; }
int password_hash_needs_rehash(const password_hash_t* hash, password_hash_algorithm_t algorithm) { (void)hash; (void)algorithm; return 0; }

int crypto_generate_salt(void* salt, size_t salt_len) { (void)salt; (void)salt_len; return SUCCESS; }
char* crypto_generate_salt_hex(size_t salt_len) { (void)salt_len; return strdup("random_salt"); }

aes_context_t* aes_context_create(aes_mode_t mode, const void* key, size_t key_len) { (void)mode; (void)key; (void)key_len; return NULL; }
void aes_context_destroy(aes_context_t* ctx) { (void)ctx; }
int aes_encrypt(aes_context_t* ctx, const void* plaintext, size_t plaintext_len, void* ciphertext, size_t* ciphertext_len, const void* iv) { (void)ctx; (void)plaintext; (void)plaintext_len; (void)ciphertext; (void)ciphertext_len; (void)iv; return SUCCESS; }
int aes_decrypt(aes_context_t* ctx, const void* ciphertext, size_t ciphertext_len, void* plaintext, size_t* plaintext_len, const void* iv) { (void)ctx; (void)ciphertext; (void)ciphertext_len; (void)plaintext; (void)plaintext_len; (void)iv; return SUCCESS; }

rsa_keypair_t* rsa_keypair_generate(rsa_key_size_t key_size) { (void)key_size; return NULL; }
void rsa_keypair_destroy(rsa_keypair_t* keypair) { (void)keypair; }
rsa_public_key_t* rsa_keypair_get_public(const rsa_keypair_t* keypair) { (void)keypair; return NULL; }
rsa_private_key_t* rsa_keypair_get_private(const rsa_keypair_t* keypair) { (void)keypair; return NULL; }
char* rsa_public_key_export_pem(const rsa_public_key_t* key) { (void)key; return strdup("-----BEGIN PUBLIC KEY-----"); }
char* rsa_private_key_export_pem(const rsa_private_key_t* key) { (void)key; return strdup("-----BEGIN PRIVATE KEY-----"); }
rsa_public_key_t* rsa_public_key_import_pem(const char* pem) { (void)pem; return NULL; }
rsa_private_key_t* rsa_private_key_import_pem(const char* pem) { (void)pem; return NULL; }
void rsa_public_key_destroy(rsa_public_key_t* key) { (void)key; }
void rsa_private_key_destroy(rsa_private_key_t* key) { (void)key; }

int rsa_encrypt(const rsa_public_key_t* key, const void* plaintext, size_t plaintext_len, void* ciphertext, size_t* ciphertext_len) { (void)key; (void)plaintext; (void)plaintext_len; (void)ciphertext; (void)ciphertext_len; return SUCCESS; }
int rsa_decrypt(const rsa_private_key_t* key, const void* ciphertext, size_t ciphertext_len, void* plaintext, size_t* plaintext_len) { (void)key; (void)ciphertext; (void)ciphertext_len; (void)plaintext; (void)plaintext_len; return SUCCESS; }

int crypto_sign(signature_algorithm_t algorithm, const rsa_private_key_t* key, const void* data, size_t data_len, void* signature, size_t* sig_len) { (void)algorithm; (void)key; (void)data; (void)data_len; (void)signature; (void)sig_len; return SUCCESS; }
int crypto_verify(signature_algorithm_t algorithm, const rsa_public_key_t* key, const void* data, size_t data_len, const void* signature, size_t sig_len) { (void)algorithm; (void)key; (void)data; (void)data_len; (void)signature; (void)sig_len; return SUCCESS; }

int crypto_random_bytes(void* buffer, size_t length) { (void)buffer; (void)length; return SUCCESS; }
uint32_t crypto_random_uint32(void) { return 42; }
uint64_t crypto_random_uint64(void) { return 42; }
int crypto_random_range(int min, int max) { (void)max; return min; }

int crypto_kdf_pbkdf2(const void* password, size_t password_len, const void* salt, size_t salt_len, int iterations, hash_algorithm_t hash_alg, void* output, size_t output_len) { (void)password; (void)password_len; (void)salt; (void)salt_len; (void)iterations; (void)hash_alg; (void)output; (void)output_len; return SUCCESS; }

int crypto_hmac(hash_algorithm_t algorithm, const void* key, size_t key_len, const void* data, size_t data_len, void* output, size_t* output_len) { (void)algorithm; (void)key; (void)key_len; (void)data; (void)data_len; (void)output; (void)output_len; return SUCCESS; }
char* crypto_hmac_hex(hash_algorithm_t algorithm, const void* key, size_t key_len, const void* data, size_t data_len) { (void)algorithm; (void)key; (void)key_len; (void)data; (void)data_len; return strdup("hmac_hex"); }
