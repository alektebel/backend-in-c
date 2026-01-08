#ifndef WEBSERVER_H
#define WEBSERVER_H

#include "common.h"
#include "http_parser.h"
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX_CONNECTIONS 100
#define BUFFER_SIZE 8192

typedef struct webserver webserver_t;

// Request handler callback
typedef void (*request_handler_t)(const http_request_t* request, http_response_t* response, void* user_data);

// Webserver functions
webserver_t* webserver_create(int port);
void webserver_destroy(webserver_t* server);
int webserver_set_handler(webserver_t* server, request_handler_t handler, void* user_data);
int webserver_start(webserver_t* server);
void webserver_stop(webserver_t* server);
int webserver_is_running(const webserver_t* server);

#endif // WEBSERVER_H
