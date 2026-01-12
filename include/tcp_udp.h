#ifndef TCP_UDP_H
#define TCP_UDP_H

#include "common.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

// Socket types
typedef enum {
    SOCKET_TCP,
    SOCKET_UDP
} socket_type_t;

// Socket states
typedef enum {
    SOCKET_STATE_CLOSED = 0,
    SOCKET_STATE_LISTENING,
    SOCKET_STATE_CONNECTED,
    SOCKET_STATE_ERROR
} socket_state_t;

// Socket family
typedef enum {
    SOCKET_FAMILY_IPV4 = AF_INET,
    SOCKET_FAMILY_IPV6 = AF_INET6,
    SOCKET_FAMILY_UNSPEC = AF_UNSPEC
} socket_family_t;

// Socket options
typedef enum {
    SOCKET_OPT_REUSEADDR,
    SOCKET_OPT_REUSEPORT,
    SOCKET_OPT_KEEPALIVE,
    SOCKET_OPT_NODELAY,
    SOCKET_OPT_RCVBUF,
    SOCKET_OPT_SNDBUF,
    SOCKET_OPT_RCVTIMEO,
    SOCKET_OPT_SNDTIMEO,
    SOCKET_OPT_BROADCAST
} socket_option_t;

// Multiplexer types
typedef enum {
    MULTIPLEXER_SELECT,
    MULTIPLEXER_POLL,
    MULTIPLEXER_EPOLL,
    MULTIPLEXER_KQUEUE
} multiplexer_type_t;

// Event flags
#define EVENT_READ  0x01
#define EVENT_WRITE 0x02
#define EVENT_ERROR 0x04
#define EVENT_HUP   0x08

// Forward declarations
typedef struct socket_multiplexer_t socket_multiplexer_t;
typedef struct tcp_connection_t tcp_connection_t;
typedef struct address_info_t address_info_t;
typedef struct socket_address_t socket_address_t;
typedef struct socket_event_t socket_event_t;

// Socket structure
typedef struct {
    int fd;                      // File descriptor
    int type;                    // SOCK_STREAM or SOCK_DGRAM
    int protocol;                // TCP or UDP
    char local_addr[256];
    int local_port;
    char remote_addr[256];
    int remote_port;
    bool is_nonblocking;
    int send_timeout_ms;
    int recv_timeout_ms;
} socket_info_t;

// Socket address structure
struct socket_address_t {
    union {
        struct sockaddr sa;
        struct sockaddr_in sin;
        struct sockaddr_in6 sin6;
        struct sockaddr_storage ss;
    } addr;
    socklen_t addr_len;
};

// Address info structure
struct address_info_t {
    socket_family_t family;
    int socktype;
    int protocol;
    socket_address_t address;
    char host[256];
    char service[32];
    struct address_info_t* next;
};

// Socket event structure
struct socket_event_t {
    int sockfd;
    uint32_t events;
    void* user_data;
};

// TCP connection structure
struct tcp_connection_t {
    int sockfd;
    socket_address_t remote_addr;
    bool is_active;
};

// TCP Server configuration
typedef struct {
    const char* host;
    int port;
    int backlog;
    bool reuse_addr;
    bool reuse_port;
    bool nodelay;           // TCP_NODELAY (disable Nagle's algorithm)
    int send_buffer_size;   // SO_SNDBUF
    int recv_buffer_size;   // SO_RCVBUF
} tcp_server_config_t;

// TCP Client configuration
typedef struct {
    const char* host;
    int port;
    int connect_timeout_ms;
    bool keepalive;
    int keepalive_idle;
    int keepalive_interval;
    int keepalive_count;
} tcp_client_config_t;

// UDP socket configuration
typedef struct {
    const char* bind_address;
    int port;
    bool broadcast_enabled;
    bool multicast_enabled;
    const char* multicast_group;
    int ttl;
    int receive_buffer_size;
    int send_buffer_size;
} udp_config_t;

// TCP server structure
typedef struct tcp_server_t tcp_server_t;

// TCP client structure
typedef struct tcp_client_t tcp_client_t;

// UDP socket structure
typedef struct udp_socket udp_socket_t;

// Connection structure
typedef struct {
    int sockfd;
    struct sockaddr_storage addr;
    socklen_t addr_len;
    bool is_connected;
    time_t last_activity;
} connection_t;

// Socket options
typedef struct {
    int reuse_addr;
    int reuse_port;
    int keepalive;
    int nodelay;
    int recv_buffer_size;
    int send_buffer_size;
    int recv_timeout_ms;
    int send_timeout_ms;
} socket_options_t;

// ==================== TCP Server/Client ====================

/**
 * Create a TCP server socket bound to a specific port
 * Based on Beej's Guide Section 6.1-6.3
 */
tcp_server_t* tcp_server_create(const char* host, const char* port, int backlog);

/**
 * Accept a connection on a TCP server socket
 * Returns a new socket file descriptor for the connected client
 */
int tcp_server_accept(tcp_server_t* server, tcp_connection_t** client);

/**
 * Start TCP server listening on specified port
 */
int tcp_server_listen(tcp_server_t* server);

/**
 * Stop TCP server and close all connections
 */
void tcp_server_stop(tcp_server_t* server);

/**
 * Destroy TCP server and free resources
 */
void tcp_server_destroy(tcp_server_t* server);

/**
 * Create a TCP client connection
 */
tcp_client_t* tcp_client_create(const char* host, int port);

/**
 * Connect to remote TCP server
 */
int tcp_client_connect(tcp_client_t* client);

/**
 * Send data over TCP connection
 */
ssize_t tcp_send(tcp_connection_t* conn, const void* data, size_t len);

/**
 * Receive data from TCP connection
 */
ssize_t tcp_recv(tcp_connection_t* conn, void* buffer, size_t len);

/**
 * Close TCP connection
 */
void tcp_close(tcp_connection_t* conn);

/**
 * Destroy TCP client and free resources
 */
void tcp_client_destroy(tcp_client_t* client);

// =============================================================================
// UDP Socket Operations (Beej's Guide Chapter 6)
// =============================================================================

/**
 * Create UDP socket for datagram communication
 */
udp_socket_t* udp_socket_create(socket_family_t family);

/**
 * Bind UDP socket to specific address and port
 */
int udp_bind(udp_socket_t* socket, const char* address, int port);

/**
 * Send UDP datagram to specific destination
 */
ssize_t udp_sendto(udp_socket_t* socket, const void* data, size_t len, 
                   const char* dest_addr, int dest_port);

/**
 * Receive UDP datagram and get sender information
 */
ssize_t udp_recvfrom(udp_socket_t* socket, void* buffer, size_t len,
                     char* src_addr, size_t addr_len, int* src_port);

/**
 * Connect UDP socket to remote address (for connected mode)
 */
int udp_connect(udp_socket_t* socket, const char* address, int port);

/**
 * Send data on connected UDP socket
 */
ssize_t udp_send(udp_socket_t* socket, const void* data, size_t len);

/**
 * Receive data on connected UDP socket
 */
ssize_t udp_recv(udp_socket_t* socket, void* buffer, size_t len);

/**
 * Close UDP socket
 */
void udp_close(udp_socket_t* socket);

/**
 * Destroy UDP socket and free resources
 */
void udp_socket_destroy(udp_socket_t* socket);

// =============================================================================
// Address Resolution (Beej's Guide Chapter 5.1)
// =============================================================================

/**
 * Resolve hostname to address information
 */
int resolve_address(const char* hostname, const char* service, 
                   address_info_t** results, int socktype);

/**
 * Free address information list
 */
void free_address_info(address_info_t* info);

/**
 * Convert address to string representation
 */
int address_to_string(const socket_address_t* addr, char* buffer, size_t len);

/**
 * Convert string to socket address
 */
int string_to_address(const char* str, int port, socket_address_t* addr);

// =============================================================================
// Socket Options (Beej's Guide Chapter 7.2)
// =============================================================================

/**
 * Set socket option
 */
int socket_set_option(int sockfd, int level, socket_option_t option, 
                     const void* value, size_t len);

/**
 * Get socket option
 */
int socket_get_option(int sockfd, int level, socket_option_t option, 
                     void* value, size_t* len);

/**
 * Set socket to non-blocking mode
 */
int socket_set_nonblocking(int sockfd, bool nonblocking);

/**
 * Set socket reuse address option
 */
int socket_set_reuseaddr(int sockfd, bool reuse);

/**
 * Set socket keep-alive option
 */
int socket_set_keepalive(int sockfd, bool keepalive);

/**
 * Set TCP no-delay option (disable Nagle's algorithm)
 */
int socket_set_nodelay(int sockfd, bool nodelay);

/**
 * Set socket receive timeout
 */
int socket_set_recv_timeout(int sockfd, int timeout_ms);

/**
 * Set socket send timeout
 */
int socket_set_send_timeout(int sockfd, int timeout_ms);

/**
 * Set socket receive buffer size
 */
int socket_set_rcvbuf(int sockfd, int size);

/**
 * Set socket send buffer size
 */
int socket_set_sndbuf(int sockfd, int size);

// =============================================================================
// I/O Multiplexing (Beej's Guide Chapter 7.3)
// =============================================================================

/**
 * Create multiplexer for handling multiple sockets
 */
socket_multiplexer_t* multiplexer_create(multiplexer_type_t type);

/**
 * Add socket to multiplexer
 */
int multiplexer_add(socket_multiplexer_t* mux, int sockfd, uint32_t events, void* user_data);

/**
 * Modify socket events in multiplexer
 */
int multiplexer_modify(socket_multiplexer_t* mux, int sockfd, uint32_t events);

/**
 * Remove socket from multiplexer
 */
int multiplexer_remove(socket_multiplexer_t* mux, int sockfd);

/**
 * Wait for events on monitored sockets
 */
int multiplexer_wait(socket_multiplexer_t* mux, socket_event_t* events, 
                    int max_events, int timeout_ms);

/**
 * Destroy multiplexer and free resources
 */
void multiplexer_destroy(socket_multiplexer_t* mux);

// =============================================================================
// High-Level Utilities
// =============================================================================

/**
 * Simple TCP echo server for testing
 */
int tcp_echo_server(int port, int backlog);

/**
 * Simple TCP client that sends and receives data
 */
int tcp_echo_client(const char* host, int port, const char* message);

/**
 * Simple UDP echo server for testing
 */
int udp_echo_server(int port);

/**
 * Simple UDP client that sends and receives datagrams
 */
int udp_echo_client(const char* host, int port, const char* message);

/**
 * Get socket error string
 */
const char* socket_error_string(int error_code);

/**
 * Get last socket error
 */
int socket_get_last_error(void);

#endif // TCP_UDP_H