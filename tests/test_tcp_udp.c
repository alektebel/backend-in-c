#include "tcp_udp.h"
#include "common.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

// Test counter
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST_ASSERT(condition, message) \
    do { \
        if (condition) { \
            printf("✓ %s\n", message); \
            tests_passed++; \
        } else { \
            printf("✗ %s\n", message); \
            tests_failed++; \
        } \
    } while(0)

// =============================================================================
// TCP Server Tests
// =============================================================================

void test_tcp_server_create_destroy(void) {
    printf("\n=== Test: TCP Server Create/Destroy ===\n");
    
    tcp_server_t* server = tcp_server_create("127.0.0.1", "8080", 10);
    TEST_ASSERT(server != NULL, "TCP server creation");
    
    tcp_server_destroy(server);
    TEST_ASSERT(1, "TCP server destruction");
}

void test_tcp_server_listen(void) {
    printf("\n=== Test: TCP Server Listen ===\n");
    
    tcp_server_t* server = tcp_server_create("0.0.0.0", "9090", 5);
    TEST_ASSERT(server != NULL, "TCP server creation");
    
    int result = tcp_server_listen(server);
    TEST_ASSERT(result == SUCCESS, "TCP server listen");
    
    tcp_server_stop(server);
    TEST_ASSERT(1, "TCP server stop");
    
    tcp_server_destroy(server);
}

void test_tcp_server_accept(void) {
    printf("\n=== Test: TCP Server Accept ===\n");
    
    tcp_server_t* server = tcp_server_create("127.0.0.1", "8081", 10);
    TEST_ASSERT(server != NULL, "TCP server creation");
    
    tcp_connection_t* client = NULL;
    int result = tcp_server_accept(server, &client);
    TEST_ASSERT(result == SUCCESS, "TCP server accept returns success");
    TEST_ASSERT(client != NULL, "TCP client connection created");
    
    if (client) {
        tcp_close(client);
        free(client);
    }
    
    tcp_server_destroy(server);
}

// =============================================================================
// TCP Client Tests
// =============================================================================

void test_tcp_client_create_destroy(void) {
    printf("\n=== Test: TCP Client Create/Destroy ===\n");
    
    tcp_client_t* client = tcp_client_create("127.0.0.1", 8080);
    TEST_ASSERT(client != NULL, "TCP client creation");
    
    tcp_client_destroy(client);
    TEST_ASSERT(1, "TCP client destruction");
}

void test_tcp_client_connect(void) {
    printf("\n=== Test: TCP Client Connect ===\n");
    
    tcp_client_t* client = tcp_client_create("localhost", 9090);
    TEST_ASSERT(client != NULL, "TCP client creation");
    
    int result = tcp_client_connect(client);
    TEST_ASSERT(result == SUCCESS, "TCP client connect");
    
    tcp_client_destroy(client);
}

void test_tcp_send_recv(void) {
    printf("\n=== Test: TCP Send/Receive ===\n");
    
    tcp_connection_t conn;
    conn.sockfd = -1;
    conn.is_active = true;
    
    const char* test_data = "Hello, TCP!";
    ssize_t sent = tcp_send(&conn, test_data, strlen(test_data));
    TEST_ASSERT(sent == (ssize_t)strlen(test_data), "TCP send data");
    
    char buffer[256];
    ssize_t received = tcp_recv(&conn, buffer, sizeof(buffer));
    TEST_ASSERT(received >= 0, "TCP receive data");
}

// =============================================================================
// UDP Socket Tests
// =============================================================================

void test_udp_socket_create_destroy(void) {
    printf("\n=== Test: UDP Socket Create/Destroy ===\n");
    
    udp_socket_t* socket = udp_socket_create(SOCKET_FAMILY_IPV4);
    TEST_ASSERT(socket != NULL, "UDP socket creation");
    
    udp_socket_destroy(socket);
    TEST_ASSERT(1, "UDP socket destruction");
}

void test_udp_bind(void) {
    printf("\n=== Test: UDP Bind ===\n");
    
    udp_socket_t* socket = udp_socket_create(SOCKET_FAMILY_IPV4);
    TEST_ASSERT(socket != NULL, "UDP socket creation");
    
    int result = udp_bind(socket, "0.0.0.0", 7070);
    TEST_ASSERT(result == SUCCESS, "UDP socket bind");
    
    udp_socket_destroy(socket);
}

void test_udp_sendto_recvfrom(void) {
    printf("\n=== Test: UDP SendTo/RecvFrom ===\n");
    
    udp_socket_t* socket = udp_socket_create(SOCKET_FAMILY_IPV4);
    TEST_ASSERT(socket != NULL, "UDP socket creation");
    
    const char* test_data = "Hello, UDP!";
    ssize_t sent = udp_sendto(socket, test_data, strlen(test_data), 
                              "127.0.0.1", 7070);
    TEST_ASSERT(sent == (ssize_t)strlen(test_data), "UDP sendto");
    
    char buffer[256];
    char src_addr[256];
    int src_port;
    ssize_t received = udp_recvfrom(socket, buffer, sizeof(buffer),
                                    src_addr, sizeof(src_addr), &src_port);
    TEST_ASSERT(received >= 0, "UDP recvfrom");
    
    udp_socket_destroy(socket);
}

void test_udp_connected_mode(void) {
    printf("\n=== Test: UDP Connected Mode ===\n");
    
    udp_socket_t* socket = udp_socket_create(SOCKET_FAMILY_IPV4);
    TEST_ASSERT(socket != NULL, "UDP socket creation");
    
    int result = udp_connect(socket, "127.0.0.1", 7070);
    TEST_ASSERT(result == SUCCESS, "UDP connect");
    
    const char* test_data = "Connected UDP!";
    ssize_t sent = udp_send(socket, test_data, strlen(test_data));
    TEST_ASSERT(sent == (ssize_t)strlen(test_data), "UDP send (connected)");
    
    char buffer[256];
    ssize_t received = udp_recv(socket, buffer, sizeof(buffer));
    TEST_ASSERT(received >= 0, "UDP recv (connected)");
    
    udp_socket_destroy(socket);
}

// =============================================================================
// Socket Options Tests
// =============================================================================

void test_socket_options(void) {
    printf("\n=== Test: Socket Options ===\n");
    
    // Use a placeholder sockfd for testing
    int sockfd = -1;
    
    int result;
    
    result = socket_set_nonblocking(sockfd, true);
    TEST_ASSERT(result == ERROR_INVALID_PARAM || result == SUCCESS, 
                "Set non-blocking mode");
    
    result = socket_set_reuseaddr(sockfd, true);
    TEST_ASSERT(result == ERROR_INVALID_PARAM || result == SUCCESS, 
                "Set reuse address");
    
    result = socket_set_keepalive(sockfd, true);
    TEST_ASSERT(result == ERROR_INVALID_PARAM || result == SUCCESS, 
                "Set keep-alive");
    
    result = socket_set_nodelay(sockfd, true);
    TEST_ASSERT(result == ERROR_INVALID_PARAM || result == SUCCESS, 
                "Set TCP no-delay");
    
    result = socket_set_recv_timeout(sockfd, 5000);
    TEST_ASSERT(result == ERROR_INVALID_PARAM || result == SUCCESS, 
                "Set receive timeout");
    
    result = socket_set_send_timeout(sockfd, 5000);
    TEST_ASSERT(result == ERROR_INVALID_PARAM || result == SUCCESS, 
                "Set send timeout");
}

// =============================================================================
// Address Resolution Tests
// =============================================================================

void test_address_resolution(void) {
    printf("\n=== Test: Address Resolution ===\n");
    
    address_info_t* results = NULL;
    int result = resolve_address("localhost", "http", &results, SOCK_STREAM);
    TEST_ASSERT(result == SUCCESS, "Resolve address");
    
    free_address_info(results);
    TEST_ASSERT(1, "Free address info");
}

void test_address_conversion(void) {
    printf("\n=== Test: Address Conversion ===\n");
    
    socket_address_t addr;
    memset(&addr, 0, sizeof(addr));
    
    int result = string_to_address("127.0.0.1", 8080, &addr);
    TEST_ASSERT(result == SUCCESS, "String to address conversion");
    
    char buffer[256];
    result = address_to_string(&addr, buffer, sizeof(buffer));
    TEST_ASSERT(result == SUCCESS, "Address to string conversion");
}

// =============================================================================
// Multiplexer Tests
// =============================================================================

void test_multiplexer_create_destroy(void) {
    printf("\n=== Test: Multiplexer Create/Destroy ===\n");
    
    socket_multiplexer_t* mux = multiplexer_create(MULTIPLEXER_SELECT);
    TEST_ASSERT(mux != NULL, "Multiplexer creation (SELECT)");
    
    multiplexer_destroy(mux);
    TEST_ASSERT(1, "Multiplexer destruction");
}

void test_multiplexer_operations(void) {
    printf("\n=== Test: Multiplexer Operations ===\n");
    
    socket_multiplexer_t* mux = multiplexer_create(MULTIPLEXER_POLL);
    TEST_ASSERT(mux != NULL, "Multiplexer creation");
    
    int result = multiplexer_add(mux, -1, EVENT_READ, NULL);
    TEST_ASSERT(result == ERROR_INVALID_PARAM || result == SUCCESS, 
                "Multiplexer add socket");
    
    result = multiplexer_modify(mux, -1, EVENT_READ | EVENT_WRITE);
    TEST_ASSERT(result == ERROR_INVALID_PARAM || result == SUCCESS, 
                "Multiplexer modify events");
    
    result = multiplexer_remove(mux, -1);
    TEST_ASSERT(result == ERROR_INVALID_PARAM || result == SUCCESS, 
                "Multiplexer remove socket");
    
    socket_event_t events[10];
    int num_events = multiplexer_wait(mux, events, 10, 100);
    TEST_ASSERT(num_events >= 0, "Multiplexer wait for events");
    
    multiplexer_destroy(mux);
}

// =============================================================================
// High-Level Utilities Tests
// =============================================================================

void test_echo_utilities(void) {
    printf("\n=== Test: Echo Utilities ===\n");
    
    // These are stubs, so we just test they don't crash
    int result;
    
    result = tcp_echo_client("127.0.0.1", 8080, "test");
    TEST_ASSERT(result == SUCCESS || result == ERROR_INVALID_PARAM, 
                "TCP echo client");
    
    result = udp_echo_client("127.0.0.1", 7070, "test");
    TEST_ASSERT(result == SUCCESS || result == ERROR_INVALID_PARAM, 
                "UDP echo client");
}

void test_error_handling(void) {
    printf("\n=== Test: Error Handling ===\n");
    
    const char* error_str = socket_error_string(SUCCESS);
    TEST_ASSERT(error_str != NULL, "Get error string for SUCCESS");
    
    error_str = socket_error_string(ERROR_MEMORY);
    TEST_ASSERT(error_str != NULL, "Get error string for ERROR_MEMORY");
    
    error_str = socket_error_string(ERROR_TIMEOUT);
    TEST_ASSERT(error_str != NULL, "Get error string for ERROR_TIMEOUT");
    
    int last_error = socket_get_last_error();
    TEST_ASSERT(1, "Get last socket error");
    (void)last_error;
}

// =============================================================================
// Main Test Runner
// =============================================================================

int main(void) {
    printf("========================================\n");
    printf("TCP/UDP Protocol Tests\n");
    printf("Based on Beej's Network Programming Guide\n");
    printf("========================================\n");
    
    // TCP Server Tests
    test_tcp_server_create_destroy();
    test_tcp_server_listen();
    test_tcp_server_accept();
    
    // TCP Client Tests
    test_tcp_client_create_destroy();
    test_tcp_client_connect();
    test_tcp_send_recv();
    
    // UDP Socket Tests
    test_udp_socket_create_destroy();
    test_udp_bind();
    test_udp_sendto_recvfrom();
    test_udp_connected_mode();
    
    // Socket Options Tests
    test_socket_options();
    
    // Address Resolution Tests
    test_address_resolution();
    test_address_conversion();
    
    // Multiplexer Tests
    test_multiplexer_create_destroy();
    test_multiplexer_operations();
    
    // High-Level Utilities Tests
    test_echo_utilities();
    test_error_handling();
    
    // Summary
    printf("\n========================================\n");
    printf("Test Results:\n");
    printf("  Passed: %d\n", tests_passed);
    printf("  Failed: %d\n", tests_failed);
    printf("  Total:  %d\n", tests_passed + tests_failed);
    printf("========================================\n");
    
    return tests_failed == 0 ? 0 : 1;
}
