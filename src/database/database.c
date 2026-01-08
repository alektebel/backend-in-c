#include "database.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#define WAL_MAGIC 0x57414C00  // "WAL\0"
#define MAX_KEY_SIZE 256
#define MAX_VALUE_SIZE (1024 * 1024)  // 1MB

// WAL entry types
typedef enum {
    WAL_ENTRY_PUT,
    WAL_ENTRY_DELETE,
    WAL_ENTRY_COMMIT,
    WAL_ENTRY_ROLLBACK
} wal_entry_type_t;

// WAL entry
typedef struct {
    uint32_t magic;
    wal_entry_type_t type;
    uint64_t txn_id;
    uint32_t key_size;
    uint32_t value_size;
    char data[];  // key + value
} wal_entry_t;

// Hash table entry
typedef struct hash_entry {
    char* key;
    void* value;
    size_t value_size;
    struct hash_entry* next;
} hash_entry_t;

// Hash table
typedef struct {
    hash_entry_t** buckets;
    size_t bucket_count;
    size_t size;
} hash_table_t;

struct database {
    char* data_dir;
    hash_table_t* table;
    int wal_fd;
    pthread_rwlock_t lock;
    uint64_t next_txn_id;
    int is_open;
};

struct transaction {
    database_t* db;
    uint64_t txn_id;
    isolation_level_t level;
    hash_table_t* writes;  // Local write set
    int committed;
};

static uint32_t hash_string(const char* str) {
    uint32_t hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

static hash_table_t* hash_table_create(size_t bucket_count) {
    hash_table_t* table = safe_calloc(1, sizeof(hash_table_t));
    table->bucket_count = bucket_count;
    table->buckets = safe_calloc(bucket_count, sizeof(hash_entry_t*));
    return table;
}

static void hash_table_destroy(hash_table_t* table) {
    if (!table) return;
    
    for (size_t i = 0; i < table->bucket_count; i++) {
        hash_entry_t* entry = table->buckets[i];
        while (entry) {
            hash_entry_t* next = entry->next;
            safe_free((void**)&entry->key);
            safe_free((void**)&entry->value);
            safe_free((void**)&entry);
            entry = next;
        }
    }
    
    safe_free((void**)&table->buckets);
    safe_free((void**)&table);
}

static int hash_table_put(hash_table_t* table, const char* key, const void* value, size_t value_size) {
    if (!table || !key) return ERROR_INVALID_PARAM;
    
    uint32_t hash = hash_string(key);
    size_t bucket = hash % table->bucket_count;
    
    // Check if key exists
    hash_entry_t* entry = table->buckets[bucket];
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            // Update existing entry
            safe_free((void**)&entry->value);
            entry->value = safe_malloc(value_size);
            memcpy(entry->value, value, value_size);
            entry->value_size = value_size;
            return SUCCESS;
        }
        entry = entry->next;
    }
    
    // Create new entry
    entry = safe_calloc(1, sizeof(hash_entry_t));
    entry->key = safe_strdup(key);
    entry->value = safe_malloc(value_size);
    memcpy(entry->value, value, value_size);
    entry->value_size = value_size;
    entry->next = table->buckets[bucket];
    table->buckets[bucket] = entry;
    table->size++;
    
    return SUCCESS;
}

static int hash_table_get(hash_table_t* table, const char* key, void** value, size_t* value_size) {
    if (!table || !key) return ERROR_INVALID_PARAM;
    
    uint32_t hash = hash_string(key);
    size_t bucket = hash % table->bucket_count;
    
    hash_entry_t* entry = table->buckets[bucket];
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            if (value) {
                *value = safe_malloc(entry->value_size);
                memcpy(*value, entry->value, entry->value_size);
            }
            if (value_size) {
                *value_size = entry->value_size;
            }
            return SUCCESS;
        }
        entry = entry->next;
    }
    
    return ERROR_NOT_FOUND;
}

static int hash_table_delete(hash_table_t* table, const char* key) {
    if (!table || !key) return ERROR_INVALID_PARAM;
    
    uint32_t hash = hash_string(key);
    size_t bucket = hash % table->bucket_count;
    
    hash_entry_t* entry = table->buckets[bucket];
    hash_entry_t* prev = NULL;
    
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            if (prev) {
                prev->next = entry->next;
            } else {
                table->buckets[bucket] = entry->next;
            }
            
            safe_free((void**)&entry->key);
            safe_free((void**)&entry->value);
            safe_free((void**)&entry);
            table->size--;
            return SUCCESS;
        }
        prev = entry;
        entry = entry->next;
    }
    
    return ERROR_NOT_FOUND;
}

database_t* database_create(const char* data_dir) {
    database_t* db = safe_calloc(1, sizeof(database_t));
    db->data_dir = safe_strdup(data_dir);
    db->table = hash_table_create(1024);
    db->wal_fd = -1;
    pthread_rwlock_init(&db->lock, NULL);
    db->next_txn_id = 1;
    return db;
}

void database_destroy(database_t* db) {
    if (!db) return;
    
    if (db->is_open) {
        database_close(db);
    }
    
    safe_free((void**)&db->data_dir);
    hash_table_destroy(db->table);
    pthread_rwlock_destroy(&db->lock);
    safe_free((void**)&db);
}

int database_open(database_t* db) {
    if (!db || db->is_open) return ERROR_INVALID_PARAM;
    
    // Create data directory
    mkdir(db->data_dir, 0755);
    
    // Open WAL file
    char wal_path[1024];
    snprintf(wal_path, sizeof(wal_path), "%s/wal.log", db->data_dir);
    db->wal_fd = open(wal_path, O_CREAT | O_RDWR | O_APPEND, 0644);
    if (db->wal_fd < 0) {
        return ERROR_IO;
    }
    
    // Recover from WAL if needed
    database_recover(db);
    
    db->is_open = 1;
    return SUCCESS;
}

int database_close(database_t* db) {
    if (!db || !db->is_open) return ERROR_INVALID_PARAM;
    
    database_checkpoint(db);
    
    if (db->wal_fd >= 0) {
        close(db->wal_fd);
        db->wal_fd = -1;
    }
    
    db->is_open = 0;
    return SUCCESS;
}

int database_put(database_t* db, const char* key, const void* value, size_t value_size) {
    if (!db || !key || !value || value_size == 0) {
        return ERROR_INVALID_PARAM;
    }
    
    pthread_rwlock_wrlock(&db->lock);
    
    // Write to WAL
    if (db->wal_fd >= 0) {
        size_t key_size = strlen(key) + 1;
        size_t entry_size = sizeof(wal_entry_t) + key_size + value_size;
        wal_entry_t* entry = safe_malloc(entry_size);
        
        entry->magic = WAL_MAGIC;
        entry->type = WAL_ENTRY_PUT;
        entry->txn_id = 0;  // Non-transactional
        entry->key_size = key_size;
        entry->value_size = value_size;
        memcpy(entry->data, key, key_size);
        memcpy(entry->data + key_size, value, value_size);
        
        write(db->wal_fd, entry, entry_size);
        fsync(db->wal_fd);
        
        safe_free((void**)&entry);
    }
    
    int result = hash_table_put(db->table, key, value, value_size);
    
    pthread_rwlock_unlock(&db->lock);
    return result;
}

int database_get(database_t* db, const char* key, void** value, size_t* value_size) {
    if (!db || !key) return ERROR_INVALID_PARAM;
    
    pthread_rwlock_rdlock(&db->lock);
    int result = hash_table_get(db->table, key, value, value_size);
    pthread_rwlock_unlock(&db->lock);
    
    return result;
}

int database_delete(database_t* db, const char* key) {
    if (!db || !key) return ERROR_INVALID_PARAM;
    
    pthread_rwlock_wrlock(&db->lock);
    
    // Write to WAL
    if (db->wal_fd >= 0) {
        size_t key_size = strlen(key) + 1;
        size_t entry_size = sizeof(wal_entry_t) + key_size;
        wal_entry_t* entry = safe_malloc(entry_size);
        
        entry->magic = WAL_MAGIC;
        entry->type = WAL_ENTRY_DELETE;
        entry->txn_id = 0;
        entry->key_size = key_size;
        entry->value_size = 0;
        memcpy(entry->data, key, key_size);
        
        write(db->wal_fd, entry, entry_size);
        fsync(db->wal_fd);
        
        safe_free((void**)&entry);
    }
    
    int result = hash_table_delete(db->table, key);
    
    pthread_rwlock_unlock(&db->lock);
    return result;
}

int database_exists(database_t* db, const char* key) {
    if (!db || !key) return 0;
    
    pthread_rwlock_rdlock(&db->lock);
    int result = (hash_table_get(db->table, key, NULL, NULL) == SUCCESS);
    pthread_rwlock_unlock(&db->lock);
    
    return result;
}

transaction_t* database_begin_transaction(database_t* db, isolation_level_t level) {
    if (!db) return NULL;
    
    transaction_t* txn = safe_calloc(1, sizeof(transaction_t));
    txn->db = db;
    txn->level = level;
    txn->writes = hash_table_create(64);
    
    pthread_rwlock_wrlock(&db->lock);
    txn->txn_id = db->next_txn_id++;
    pthread_rwlock_unlock(&db->lock);
    
    return txn;
}

int transaction_commit(transaction_t* txn) {
    if (!txn || txn->committed) return ERROR_INVALID_PARAM;
    
    database_t* db = txn->db;
    
    pthread_rwlock_wrlock(&db->lock);
    
    // Apply all writes from transaction
    for (size_t i = 0; i < txn->writes->bucket_count; i++) {
        hash_entry_t* entry = txn->writes->buckets[i];
        while (entry) {
            hash_table_put(db->table, entry->key, entry->value, entry->value_size);
            entry = entry->next;
        }
    }
    
    txn->committed = 1;
    
    pthread_rwlock_unlock(&db->lock);
    
    hash_table_destroy(txn->writes);
    safe_free((void**)&txn);
    
    return SUCCESS;
}

int transaction_rollback(transaction_t* txn) {
    if (!txn) return ERROR_INVALID_PARAM;
    
    hash_table_destroy(txn->writes);
    safe_free((void**)&txn);
    
    return SUCCESS;
}

int transaction_put(transaction_t* txn, const char* key, const void* value, size_t value_size) {
    if (!txn || !key || !value) return ERROR_INVALID_PARAM;
    return hash_table_put(txn->writes, key, value, value_size);
}

int transaction_get(transaction_t* txn, const char* key, void** value, size_t* value_size) {
    if (!txn || !key) return ERROR_INVALID_PARAM;
    
    // Check local writes first
    int result = hash_table_get(txn->writes, key, value, value_size);
    if (result == SUCCESS) return SUCCESS;
    
    // Fall back to database
    return database_get(txn->db, key, value, value_size);
}

int transaction_delete(transaction_t* txn, const char* key) {
    if (!txn || !key) return ERROR_INVALID_PARAM;
    return hash_table_delete(txn->writes, key);
}

int database_create_index(database_t* db, const char* index_name, index_type_t type) {
    // Placeholder for index creation
    return SUCCESS;
}

int database_drop_index(database_t* db, const char* index_name) {
    // Placeholder for index removal
    return SUCCESS;
}

int database_checkpoint(database_t* db) {
    if (!db || db->wal_fd < 0) return ERROR_INVALID_PARAM;
    
    // Truncate WAL after checkpoint
    ftruncate(db->wal_fd, 0);
    lseek(db->wal_fd, 0, SEEK_SET);
    
    return SUCCESS;
}

int database_recover(database_t* db) {
    if (!db || db->wal_fd < 0) return ERROR_INVALID_PARAM;
    
    // Read and replay WAL entries
    lseek(db->wal_fd, 0, SEEK_SET);
    
    wal_entry_t header;
    while (read(db->wal_fd, &header, sizeof(wal_entry_t)) == sizeof(wal_entry_t)) {
        if (header.magic != WAL_MAGIC) break;
        
        char* data = safe_malloc(header.key_size + header.value_size);
        read(db->wal_fd, data, header.key_size + header.value_size);
        
        const char* key = data;
        
        if (header.type == WAL_ENTRY_PUT) {
            const void* value = data + header.key_size;
            hash_table_put(db->table, key, value, header.value_size);
        } else if (header.type == WAL_ENTRY_DELETE) {
            hash_table_delete(db->table, key);
        }
        
        safe_free((void**)&data);
    }
    
    return SUCCESS;
}

int database_compact(database_t* db) {
    // Placeholder for compaction logic
    return SUCCESS;
}

int database_get_stats(database_t* db, database_stats_t* stats) {
    if (!db || !stats) return ERROR_INVALID_PARAM;
    
    pthread_rwlock_rdlock(&db->lock);
    
    stats->num_keys = db->table->size;
    stats->total_size = 0;
    stats->wal_size = db->wal_fd >= 0 ? lseek(db->wal_fd, 0, SEEK_END) : 0;
    stats->num_transactions = db->next_txn_id - 1;
    
    // Calculate total size
    for (size_t i = 0; i < db->table->bucket_count; i++) {
        hash_entry_t* entry = db->table->buckets[i];
        while (entry) {
            stats->total_size += strlen(entry->key) + entry->value_size;
            entry = entry->next;
        }
    }
    
    pthread_rwlock_unlock(&db->lock);
    
    return SUCCESS;
}
