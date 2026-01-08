#include "http_parser.h"
#include <ctype.h>

const char* http_method_to_string(http_method_t method) {
    switch (method) {
        case HTTP_GET: return "GET";
        case HTTP_POST: return "POST";
        case HTTP_PUT: return "PUT";
        case HTTP_DELETE: return "DELETE";
        case HTTP_HEAD: return "HEAD";
        case HTTP_OPTIONS: return "OPTIONS";
        case HTTP_PATCH: return "PATCH";
        default: return "UNKNOWN";
    }
}

http_method_t http_method_from_string(const char* method_str) {
    if (!method_str) return HTTP_UNKNOWN;
    if (strcmp(method_str, "GET") == 0) return HTTP_GET;
    if (strcmp(method_str, "POST") == 0) return HTTP_POST;
    if (strcmp(method_str, "PUT") == 0) return HTTP_PUT;
    if (strcmp(method_str, "DELETE") == 0) return HTTP_DELETE;
    if (strcmp(method_str, "HEAD") == 0) return HTTP_HEAD;
    if (strcmp(method_str, "OPTIONS") == 0) return HTTP_OPTIONS;
    if (strcmp(method_str, "PATCH") == 0) return HTTP_PATCH;
    return HTTP_UNKNOWN;
}

http_request_t* http_request_create(void) {
    http_request_t* request = safe_calloc(1, sizeof(http_request_t));
    request->method = HTTP_UNKNOWN;
    request->version = HTTP_VERSION_UNKNOWN;
    return request;
}

void http_request_destroy(http_request_t* request) {
    if (!request) return;
    
    safe_free((void**)&request->uri);
    safe_free((void**)&request->path);
    safe_free((void**)&request->query);
    safe_free((void**)&request->body);
    
    for (size_t i = 0; i < request->header_count; i++) {
        safe_free((void**)&request->headers[i].name);
        safe_free((void**)&request->headers[i].value);
    }
    safe_free((void**)&request->headers);
    safe_free((void**)&request);
}

int http_request_parse(http_request_t* request, const char* raw_request, size_t length) {
    if (!request || !raw_request || length == 0) {
        return ERROR_INVALID_PARAM;
    }
    
    // Find the end of the request line
    const char* line_end = strstr(raw_request, "\r\n");
    if (!line_end) {
        return ERROR_INVALID_PARAM;
    }
    
    // Parse request line: METHOD URI VERSION
    char method_str[16] = {0};
    char uri[2048] = {0};
    char version_str[16] = {0};
    
    int parsed = sscanf(raw_request, "%15s %2047s %15s", method_str, uri, version_str);
    if (parsed != 3) {
        return ERROR_INVALID_PARAM;
    }
    
    request->method = http_method_from_string(method_str);
    request->uri = safe_strdup(uri);
    
    // Parse version
    if (strcmp(version_str, "HTTP/1.0") == 0) {
        request->version = HTTP_1_0;
    } else if (strcmp(version_str, "HTTP/1.1") == 0) {
        request->version = HTTP_1_1;
    } else if (strcmp(version_str, "HTTP/2.0") == 0) {
        request->version = HTTP_2_0;
    }
    
    // Parse path and query
    char* query_start = strchr(uri, '?');
    if (query_start) {
        size_t path_len = query_start - uri;
        request->path = safe_malloc(path_len + 1);
        strncpy(request->path, uri, path_len);
        request->path[path_len] = '\0';
        request->query = safe_strdup(query_start + 1);
    } else {
        request->path = safe_strdup(uri);
    }
    
    // Parse headers
    const char* header_start = line_end + 2;
    const char* headers_end = strstr(header_start, "\r\n\r\n");
    
    if (headers_end) {
        // Count headers
        const char* current = header_start;
        size_t count = 0;
        while (current < headers_end) {
            const char* next = strstr(current, "\r\n");
            if (!next || next >= headers_end) break;
            count++;
            current = next + 2;
        }
        
        if (count > 0) {
            request->headers = safe_calloc(count, sizeof(http_header_t));
            request->header_count = count;
            
            current = header_start;
            size_t idx = 0;
            while (current < headers_end && idx < count) {
                const char* next = strstr(current, "\r\n");
                if (!next || next >= headers_end) break;
                
                const char* colon = strchr(current, ':');
                if (colon && colon < next) {
                    size_t name_len = colon - current;
                    request->headers[idx].name = safe_malloc(name_len + 1);
                    strncpy(request->headers[idx].name, current, name_len);
                    request->headers[idx].name[name_len] = '\0';
                    
                    // Skip colon and spaces
                    const char* value_start = colon + 1;
                    while (value_start < next && isspace(*value_start)) {
                        value_start++;
                    }
                    
                    size_t value_len = next - value_start;
                    request->headers[idx].value = safe_malloc(value_len + 1);
                    strncpy(request->headers[idx].value, value_start, value_len);
                    request->headers[idx].value[value_len] = '\0';
                    
                    idx++;
                }
                
                current = next + 2;
            }
        }
        
        // Parse body
        const char* body_start = headers_end + 4;
        size_t remaining = length - (body_start - raw_request);
        if (remaining > 0) {
            request->body = safe_malloc(remaining + 1);
            memcpy(request->body, body_start, remaining);
            request->body[remaining] = '\0';
            request->body_length = remaining;
        }
    }
    
    return SUCCESS;
}

const char* http_request_get_header(const http_request_t* request, const char* name) {
    if (!request || !name) return NULL;
    
    for (size_t i = 0; i < request->header_count; i++) {
        if (strcasecmp(request->headers[i].name, name) == 0) {
            return request->headers[i].value;
        }
    }
    return NULL;
}

int http_request_add_header(http_request_t* request, const char* name, const char* value) {
    if (!request || !name || !value) {
        return ERROR_INVALID_PARAM;
    }
    
    request->headers = safe_realloc(request->headers, 
                                    (request->header_count + 1) * sizeof(http_header_t));
    request->headers[request->header_count].name = safe_strdup(name);
    request->headers[request->header_count].value = safe_strdup(value);
    request->header_count++;
    
    return SUCCESS;
}

http_response_t* http_response_create(int status_code, const char* status_message) {
    http_response_t* response = safe_calloc(1, sizeof(http_response_t));
    response->version = HTTP_1_1;
    response->status_code = status_code;
    response->status_message = status_message ? safe_strdup(status_message) : safe_strdup("OK");
    return response;
}

void http_response_destroy(http_response_t* response) {
    if (!response) return;
    
    safe_free((void**)&response->status_message);
    safe_free((void**)&response->body);
    
    for (size_t i = 0; i < response->header_count; i++) {
        safe_free((void**)&response->headers[i].name);
        safe_free((void**)&response->headers[i].value);
    }
    safe_free((void**)&response->headers);
    safe_free((void**)&response);
}

int http_response_add_header(http_response_t* response, const char* name, const char* value) {
    if (!response || !name || !value) {
        return ERROR_INVALID_PARAM;
    }
    
    response->headers = safe_realloc(response->headers,
                                     (response->header_count + 1) * sizeof(http_header_t));
    response->headers[response->header_count].name = safe_strdup(name);
    response->headers[response->header_count].value = safe_strdup(value);
    response->header_count++;
    
    return SUCCESS;
}

int http_response_set_body(http_response_t* response, const char* body, size_t length) {
    if (!response) {
        return ERROR_INVALID_PARAM;
    }
    
    safe_free((void**)&response->body);
    
    if (body && length > 0) {
        response->body = safe_malloc(length + 1);
        memcpy(response->body, body, length);
        response->body[length] = '\0';
        response->body_length = length;
    } else {
        response->body_length = 0;
    }
    
    return SUCCESS;
}

char* http_response_serialize(const http_response_t* response, size_t* out_length) {
    if (!response) return NULL;
    
    // Calculate required size
    size_t size = 256; // Initial size for status line
    for (size_t i = 0; i < response->header_count; i++) {
        size += strlen(response->headers[i].name) + strlen(response->headers[i].value) + 4;
    }
    size += response->body_length + 4;
    
    char* buffer = safe_malloc(size);
    size_t offset = 0;
    
    // Status line
    offset += snprintf(buffer + offset, size - offset, "HTTP/1.1 %d %s\r\n",
                      response->status_code, response->status_message);
    
    // Headers
    for (size_t i = 0; i < response->header_count; i++) {
        offset += snprintf(buffer + offset, size - offset, "%s: %s\r\n",
                          response->headers[i].name, response->headers[i].value);
    }
    
    // Empty line
    offset += snprintf(buffer + offset, size - offset, "\r\n");
    
    // Body
    if (response->body && response->body_length > 0) {
        memcpy(buffer + offset, response->body, response->body_length);
        offset += response->body_length;
    }
    
    if (out_length) {
        *out_length = offset;
    }
    
    return buffer;
}
