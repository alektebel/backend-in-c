#include "http_status.h"

// HTTP Status message lookup
const char* http_status_get_message(int status_code) {
    switch (status_code) {
        // 1xx Informational
        case 100: return "Continue";
        case 101: return "Switching Protocols";
        case 102: return "Processing";
        case 103: return "Early Hints";
        
        // 2xx Success
        case 200: return "OK";
        case 201: return "Created";
        case 202: return "Accepted";
        case 203: return "Non-Authoritative Information";
        case 204: return "No Content";
        case 205: return "Reset Content";
        case 206: return "Partial Content";
        
        // 3xx Redirection
        case 300: return "Multiple Choices";
        case 301: return "Moved Permanently";
        case 302: return "Found";
        case 303: return "See Other";
        case 304: return "Not Modified";
        case 307: return "Temporary Redirect";
        case 308: return "Permanent Redirect";
        
        // 4xx Client Errors
        case 400: return "Bad Request";
        case 401: return "Unauthorized";
        case 402: return "Payment Required";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 406: return "Not Acceptable";
        case 407: return "Proxy Authentication Required";
        case 408: return "Request Timeout";
        case 409: return "Conflict";
        case 410: return "Gone";
        case 411: return "Length Required";
        case 412: return "Precondition Failed";
        case 413: return "Payload Too Large";
        case 414: return "URI Too Long";
        case 415: return "Unsupported Media Type";
        case 416: return "Range Not Satisfiable";
        case 417: return "Expectation Failed";
        case 418: return "I'm a teapot";
        case 422: return "Unprocessable Entity";
        case 429: return "Too Many Requests";
        
        // 5xx Server Errors
        case 500: return "Internal Server Error";
        case 501: return "Not Implemented";
        case 502: return "Bad Gateway";
        case 503: return "Service Unavailable";
        case 504: return "Gateway Timeout";
        case 505: return "HTTP Version Not Supported";
        
        default: return "Unknown";
    }
}

http_status_category_t http_status_get_category(int status_code) {
    if (status_code >= 100 && status_code < 200) return HTTP_STATUS_INFORMATIONAL;
    if (status_code >= 200 && status_code < 300) return HTTP_STATUS_SUCCESS;
    if (status_code >= 300 && status_code < 400) return HTTP_STATUS_REDIRECTION;
    if (status_code >= 400 && status_code < 500) return HTTP_STATUS_CLIENT_ERROR;
    if (status_code >= 500 && status_code < 600) return HTTP_STATUS_SERVER_ERROR;
    return HTTP_STATUS_CLIENT_ERROR; // Default
}

int http_status_is_informational(int status_code) {
    return status_code >= 100 && status_code < 200;
}

int http_status_is_success(int status_code) {
    return status_code >= 200 && status_code < 300;
}

int http_status_is_redirection(int status_code) {
    return status_code >= 300 && status_code < 400;
}

int http_status_is_client_error(int status_code) {
    return status_code >= 400 && status_code < 500;
}

int http_status_is_server_error(int status_code) {
    return status_code >= 500 && status_code < 600;
}

int http_status_is_error(int status_code) {
    return http_status_is_client_error(status_code) || 
           http_status_is_server_error(status_code);
}
