#define _POSIX_C_SOURCE 200809L
#include "webserver.h"
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

struct webserver {
    int port;
    int socket_fd;
    int is_running;
    request_handler_t handler;
    void* user_data;
    pthread_t server_thread;
};

typedef struct {
    int client_fd;
    webserver_t* server;
} client_context_t;

static void* handle_client(void* arg) {
    client_context_t* ctx = (client_context_t*)arg;
    int client_fd = ctx->client_fd;
    webserver_t* server = ctx->server;
    
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        
        http_request_t* request = http_request_create();
        if (http_request_parse(request, buffer, bytes_read) == SUCCESS) {
            http_response_t* response = http_response_create(200, "OK");
            
            if (server->handler) {
                server->handler(request, response, server->user_data);
            } else {
                // Default response
                const char* default_body = "Hello from backend-in-c!";
                http_response_set_body(response, default_body, strlen(default_body));
                http_response_add_header(response, "Content-Type", "text/plain");
            }
            
            size_t response_length;
            char* response_data = http_response_serialize(response, &response_length);
            send(client_fd, response_data, response_length, 0);
            
            safe_free((void**)&response_data);
            http_response_destroy(response);
        }
        
        http_request_destroy(request);
    }
    
    close(client_fd);
    free(ctx);
    return NULL;
}

static void* server_loop(void* arg) {
    webserver_t* server = (webserver_t*)arg;
    
    while (server->is_running) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        int client_fd = accept(server->socket_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) {
            if (server->is_running) {
                perror("accept failed");
            }
            continue;
        }
        
        client_context_t* ctx = safe_malloc(sizeof(client_context_t));
        ctx->client_fd = client_fd;
        ctx->server = server;
        
        pthread_t thread;
        if (pthread_create(&thread, NULL, handle_client, ctx) != 0) {
            perror("pthread_create failed");
            close(client_fd);
            free(ctx);
            continue;
        }
        pthread_detach(thread);
    }
    
    return NULL;
}

webserver_t* webserver_create(int port) {
    webserver_t* server = safe_calloc(1, sizeof(webserver_t));
    server->port = port;
    server->socket_fd = -1;
    server->is_running = 0;
    return server;
}

void webserver_destroy(webserver_t* server) {
    if (!server) return;
    
    if (server->is_running) {
        webserver_stop(server);
    }
    
    safe_free((void**)&server);
}

int webserver_set_handler(webserver_t* server, request_handler_t handler, void* user_data) {
    if (!server) {
        return ERROR_INVALID_PARAM;
    }
    
    server->handler = handler;
    server->user_data = user_data;
    return SUCCESS;
}

int webserver_start(webserver_t* server) {
    if (!server || server->is_running) {
        return ERROR_INVALID_PARAM;
    }
    
    // Create socket
    server->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server->socket_fd < 0) {
        perror("socket creation failed");
        return ERROR_IO;
    }
    
    // Set socket options
    int opt = 1;
    if (setsockopt(server->socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        close(server->socket_fd);
        return ERROR_IO;
    }
    
    // Bind socket
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(server->port);
    
    if (bind(server->socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        close(server->socket_fd);
        return ERROR_IO;
    }
    
    // Listen
    if (listen(server->socket_fd, MAX_CONNECTIONS) < 0) {
        perror("listen failed");
        close(server->socket_fd);
        return ERROR_IO;
    }
    
    server->is_running = 1;
    
    // Start server thread
    if (pthread_create(&server->server_thread, NULL, server_loop, server) != 0) {
        perror("pthread_create failed");
        close(server->socket_fd);
        server->is_running = 0;
        return ERROR_IO;
    }
    
    printf("Web server started on port %d\n", server->port);
    return SUCCESS;
}

void webserver_stop(webserver_t* server) {
    if (!server || !server->is_running) {
        return;
    }
    
    server->is_running = 0;
    
    // Shutdown the socket to wake up accept()
    if (server->socket_fd >= 0) {
        shutdown(server->socket_fd, SHUT_RDWR);
        close(server->socket_fd);
    }
    
    pthread_join(server->server_thread, NULL);
    
    printf("Web server stopped\n");
}

int webserver_is_running(const webserver_t* server) {
    return server ? server->is_running : 0;
}
