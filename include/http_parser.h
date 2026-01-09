#ifndef HTTP_PARSER_H
#define HTTP_PARSER_H

#include "common.h"

// HTTP methods
typedef enum {
    HTTP_GET,
    HTTP_POST,
    HTTP_PUT,
    HTTP_DELETE,
    HTTP_HEAD,
    HTTP_OPTIONS,
    HTTP_PATCH,
    HTTP_UNKNOWN
} http_method_t;

// HTTP version
typedef enum {
    HTTP_1_0,
    HTTP_1_1,
    HTTP_2_0,
    HTTP_VERSION_UNKNOWN
} http_version_t;

// HTTP header
typedef struct {
    char* name;
    char* value;
} http_header_t;

// HTTP request
typedef struct {
    http_method_t method;
    http_version_t version;
    char* uri;
    char* path;
    char* query;
    http_header_t* headers;
    size_t header_count;
    char* body;
    size_t body_length;
} http_request_t;

// HTTP response
typedef struct {
    http_version_t version;
    int status_code;
    char* status_message;
    http_header_t* headers;
    size_t header_count;
    char* body;
    size_t body_length;
} http_response_t;

// HTTP parser functions
http_request_t* http_request_create(void);
void http_request_destroy(http_request_t* request);
int http_request_parse(http_request_t* request, const char* raw_request, size_t length);
const char* http_request_get_header(const http_request_t* request, const char* name);
int http_request_add_header(http_request_t* request, const char* name, const char* value);

http_response_t* http_response_create(int status_code, const char* status_message);
void http_response_destroy(http_response_t* response);
int http_response_add_header(http_response_t* response, const char* name, const char* value);
int http_response_set_body(http_response_t* response, const char* body, size_t length);
char* http_response_serialize(const http_response_t* response, size_t* out_length);

const char* http_method_to_string(http_method_t method);
http_method_t http_method_from_string(const char* method_str);

#endif // HTTP_PARSER_H
