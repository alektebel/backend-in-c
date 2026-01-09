#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

// Common error codes
typedef enum {
    SUCCESS = 0,
    ERROR_MEMORY = -1,
    ERROR_INVALID_PARAM = -2,
    ERROR_NOT_FOUND = -3,
    ERROR_ALREADY_EXISTS = -4,
    ERROR_IO = -5,
    ERROR_FULL = -6,
    ERROR_EMPTY = -7,
    ERROR_TIMEOUT = -8
} error_code_t;

// Memory allocation helpers
void* safe_malloc(size_t size);
void* safe_calloc(size_t nmemb, size_t size);
void* safe_realloc(void* ptr, size_t size);
void safe_free(void** ptr);

// String utilities
char* safe_strdup(const char* str);
int safe_strcmp(const char* s1, const char* s2);

// Time utilities
uint64_t get_timestamp_ms(void);

#endif // COMMON_H
