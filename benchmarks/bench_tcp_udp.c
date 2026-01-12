#include "tcp_udp.h"
#include "common.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

#define BENCH_ITERATIONS 100000
#define BENCH_SMALL_DATA_SIZE 64
#define BENCH_LARGE_DATA_SIZE 8192

// Timing utilities
static uint64_t get_time_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

static void print_benchmark_result(const char* name, uint64_t total_ns, int iterations) {
    double avg_ns = (double)total_ns / iterations;
    double ops_per_sec = 1000000000.0 / avg_ns;
    
    printf("%-40s: %10.2f ns/op, %12.0f ops/sec\n", 
           name, avg_ns, ops_per_sec);
}

// =============================================================================
// TCP Benchmarks
// =============================================================================

void bench_tcp_server_create_destroy(void) {
    uint64_t start = get_time_ns();
    
    for (int i = 0; i < BENCH_ITERATIONS; i++) {
        tcp_server_t* server = tcp_server_create("127.0.0.1", "8080", 10);
        tcp_server_destroy(server);
    }
    
    uint64_t elapsed = get_time_ns() - start;
    print_benchmark_result("TCP Server Create/Destroy", elapsed, BENCH_ITERATIONS);
}

void bench_tcp_client_create_destroy(void) {
    uint64_t start = get_time_ns();
    
    for (int i = 0; i < BENCH_ITERATIONS; i++) {
        tcp_client_t* client = tcp_client_create("127.0.0.1", 8080);
        tcp_client_destroy(client);
    }
    
    uint64_t elapsed = get_time_ns() - start;
    print_benchmark_result("TCP Client Create/Destroy", elapsed, BENCH_ITERATIONS);
}

void bench_tcp_send_small(void) {
    tcp_connection_t conn;
    conn.sockfd = -1;
    conn.is_active = true;
    
    char data[BENCH_SMALL_DATA_SIZE];
    memset(data, 'A', sizeof(data));
    
    uint64_t start = get_time_ns();
    
    for (int i = 0; i < BENCH_ITERATIONS; i++) {
        tcp_send(&conn, data, sizeof(data));
    }
    
    uint64_t elapsed = get_time_ns() - start;
    print_benchmark_result("TCP Send (64 bytes)", elapsed, BENCH_ITERATIONS);
}

void bench_tcp_send_large(void) {
    tcp_connection_t conn;
    conn.sockfd = -1;
    conn.is_active = true;
    
    char data[BENCH_LARGE_DATA_SIZE];
    memset(data, 'A', sizeof(data));
    
    uint64_t start = get_time_ns();
    
    for (int i = 0; i < BENCH_ITERATIONS / 10; i++) {
        tcp_send(&conn, data, sizeof(data));
    }
    
    uint64_t elapsed = get_time_ns() - start;
    print_benchmark_result("TCP Send (8KB)", elapsed, BENCH_ITERATIONS / 10);
}

void bench_tcp_recv(void) {
    tcp_connection_t conn;
    conn.sockfd = -1;
    conn.is_active = true;
    
    char buffer[BENCH_SMALL_DATA_SIZE];
    
    uint64_t start = get_time_ns();
    
    for (int i = 0; i < BENCH_ITERATIONS; i++) {
        tcp_recv(&conn, buffer, sizeof(buffer));
    }
    
    uint64_t elapsed = get_time_ns() - start;
    print_benchmark_result("TCP Receive", elapsed, BENCH_ITERATIONS);
}

// =============================================================================
// UDP Benchmarks
// =============================================================================

void bench_udp_socket_create_destroy(void) {
    uint64_t start = get_time_ns();
    
    for (int i = 0; i < BENCH_ITERATIONS; i++) {
        udp_socket_t* socket = udp_socket_create(SOCKET_FAMILY_IPV4);
        udp_socket_destroy(socket);
    }
    
    uint64_t elapsed = get_time_ns() - start;
    print_benchmark_result("UDP Socket Create/Destroy", elapsed, BENCH_ITERATIONS);
}

void bench_udp_sendto_small(void) {
    udp_socket_t* socket = udp_socket_create(SOCKET_FAMILY_IPV4);
    
    char data[BENCH_SMALL_DATA_SIZE];
    memset(data, 'B', sizeof(data));
    
    uint64_t start = get_time_ns();
    
    for (int i = 0; i < BENCH_ITERATIONS; i++) {
        udp_sendto(socket, data, sizeof(data), "127.0.0.1", 7070);
    }
    
    uint64_t elapsed = get_time_ns() - start;
    print_benchmark_result("UDP SendTo (64 bytes)", elapsed, BENCH_ITERATIONS);
    
    udp_socket_destroy(socket);
}

void bench_udp_sendto_large(void) {
    udp_socket_t* socket = udp_socket_create(SOCKET_FAMILY_IPV4);
    
    char data[BENCH_LARGE_DATA_SIZE];
    memset(data, 'B', sizeof(data));
    
    uint64_t start = get_time_ns();
    
    for (int i = 0; i < BENCH_ITERATIONS / 10; i++) {
        udp_sendto(socket, data, sizeof(data), "127.0.0.1", 7070);
    }
    
    uint64_t elapsed = get_time_ns() - start;
    print_benchmark_result("UDP SendTo (8KB)", elapsed, BENCH_ITERATIONS / 10);
    
    udp_socket_destroy(socket);
}

void bench_udp_recvfrom(void) {
    udp_socket_t* socket = udp_socket_create(SOCKET_FAMILY_IPV4);
    
    char buffer[BENCH_SMALL_DATA_SIZE];
    char src_addr[256];
    int src_port;
    
    uint64_t start = get_time_ns();
    
    for (int i = 0; i < BENCH_ITERATIONS; i++) {
        udp_recvfrom(socket, buffer, sizeof(buffer), 
                     src_addr, sizeof(src_addr), &src_port);
    }
    
    uint64_t elapsed = get_time_ns() - start;
    print_benchmark_result("UDP RecvFrom", elapsed, BENCH_ITERATIONS);
    
    udp_socket_destroy(socket);
}

// =============================================================================
// Socket Options Benchmarks
// =============================================================================

void bench_socket_set_options(void) {
    int sockfd = -1;
    
    uint64_t start = get_time_ns();
    
    for (int i = 0; i < BENCH_ITERATIONS; i++) {
        socket_set_nonblocking(sockfd, true);
        socket_set_reuseaddr(sockfd, true);
        socket_set_keepalive(sockfd, true);
        socket_set_nodelay(sockfd, true);
    }
    
    uint64_t elapsed = get_time_ns() - start;
    print_benchmark_result("Socket Set Options (4x)", elapsed, BENCH_ITERATIONS);
}

// =============================================================================
// Address Resolution Benchmarks
// =============================================================================

void bench_address_resolution(void) {
    uint64_t start = get_time_ns();
    
    for (int i = 0; i < BENCH_ITERATIONS / 100; i++) {
        address_info_t* results = NULL;
        resolve_address("localhost", "http", &results, SOCK_STREAM);
        free_address_info(results);
    }
    
    uint64_t elapsed = get_time_ns() - start;
    print_benchmark_result("Address Resolution", elapsed, BENCH_ITERATIONS / 100);
}

void bench_address_conversion(void) {
    socket_address_t addr;
    char buffer[256];
    
    uint64_t start = get_time_ns();
    
    for (int i = 0; i < BENCH_ITERATIONS; i++) {
        string_to_address("127.0.0.1", 8080, &addr);
        address_to_string(&addr, buffer, sizeof(buffer));
    }
    
    uint64_t elapsed = get_time_ns() - start;
    print_benchmark_result("Address Conversion (both ways)", elapsed, BENCH_ITERATIONS);
}

// =============================================================================
// Multiplexer Benchmarks
// =============================================================================

void bench_multiplexer_create_destroy(void) {
    uint64_t start = get_time_ns();
    
    for (int i = 0; i < BENCH_ITERATIONS / 10; i++) {
        socket_multiplexer_t* mux = multiplexer_create(MULTIPLEXER_SELECT);
        multiplexer_destroy(mux);
    }
    
    uint64_t elapsed = get_time_ns() - start;
    print_benchmark_result("Multiplexer Create/Destroy", elapsed, BENCH_ITERATIONS / 10);
}

void bench_multiplexer_operations(void) {
    socket_multiplexer_t* mux = multiplexer_create(MULTIPLEXER_POLL);
    
    uint64_t start = get_time_ns();
    
    for (int i = 0; i < BENCH_ITERATIONS; i++) {
        multiplexer_add(mux, -1, EVENT_READ, NULL);
        multiplexer_modify(mux, -1, EVENT_READ | EVENT_WRITE);
        multiplexer_remove(mux, -1);
    }
    
    uint64_t elapsed = get_time_ns() - start;
    print_benchmark_result("Multiplexer Add/Modify/Remove", elapsed, BENCH_ITERATIONS);
    
    multiplexer_destroy(mux);
}

void bench_multiplexer_wait(void) {
    socket_multiplexer_t* mux = multiplexer_create(MULTIPLEXER_EPOLL);
    socket_event_t events[10];
    
    uint64_t start = get_time_ns();
    
    for (int i = 0; i < BENCH_ITERATIONS / 100; i++) {
        multiplexer_wait(mux, events, 10, 0);
    }
    
    uint64_t elapsed = get_time_ns() - start;
    print_benchmark_result("Multiplexer Wait (no timeout)", elapsed, BENCH_ITERATIONS / 100);
    
    multiplexer_destroy(mux);
}

// =============================================================================
// Throughput Benchmarks
// =============================================================================

void bench_tcp_throughput(void) {
    printf("\n=== TCP Throughput Benchmarks ===\n");
    
    tcp_connection_t conn;
    conn.sockfd = -1;
    conn.is_active = true;
    
    // Small packets
    {
        char data[64];
        memset(data, 'X', sizeof(data));
        
        uint64_t start = get_time_ns();
        size_t total_bytes = 0;
        
        for (int i = 0; i < BENCH_ITERATIONS; i++) {
            ssize_t sent = tcp_send(&conn, data, sizeof(data));
            if (sent > 0) total_bytes += sent;
        }
        
        uint64_t elapsed = get_time_ns() - start;
        double seconds = elapsed / 1000000000.0;
        double mbps = (total_bytes / (1024.0 * 1024.0)) / seconds;
        
        printf("  TCP throughput (64B packets):  %10.2f MB/s\n", mbps);
    }
    
    // Large packets
    {
        char data[8192];
        memset(data, 'X', sizeof(data));
        
        uint64_t start = get_time_ns();
        size_t total_bytes = 0;
        
        for (int i = 0; i < BENCH_ITERATIONS / 10; i++) {
            ssize_t sent = tcp_send(&conn, data, sizeof(data));
            if (sent > 0) total_bytes += sent;
        }
        
        uint64_t elapsed = get_time_ns() - start;
        double seconds = elapsed / 1000000000.0;
        double mbps = (total_bytes / (1024.0 * 1024.0)) / seconds;
        
        printf("  TCP throughput (8KB packets):  %10.2f MB/s\n", mbps);
    }
}

void bench_udp_throughput(void) {
    printf("\n=== UDP Throughput Benchmarks ===\n");
    
    udp_socket_t* socket = udp_socket_create(SOCKET_FAMILY_IPV4);
    
    // Small packets
    {
        char data[64];
        memset(data, 'Y', sizeof(data));
        
        uint64_t start = get_time_ns();
        size_t total_bytes = 0;
        
        for (int i = 0; i < BENCH_ITERATIONS; i++) {
            ssize_t sent = udp_sendto(socket, data, sizeof(data), 
                                     "127.0.0.1", 7070);
            if (sent > 0) total_bytes += sent;
        }
        
        uint64_t elapsed = get_time_ns() - start;
        double seconds = elapsed / 1000000000.0;
        double mbps = (total_bytes / (1024.0 * 1024.0)) / seconds;
        
        printf("  UDP throughput (64B packets):  %10.2f MB/s\n", mbps);
    }
    
    // Large packets (UDP typical max 1472 bytes for IPv4)
    {
        char data[1472];
        memset(data, 'Y', sizeof(data));
        
        uint64_t start = get_time_ns();
        size_t total_bytes = 0;
        
        for (int i = 0; i < BENCH_ITERATIONS / 10; i++) {
            ssize_t sent = udp_sendto(socket, data, sizeof(data), 
                                     "127.0.0.1", 7070);
            if (sent > 0) total_bytes += sent;
        }
        
        uint64_t elapsed = get_time_ns() - start;
        double seconds = elapsed / 1000000000.0;
        double mbps = (total_bytes / (1024.0 * 1024.0)) / seconds;
        
        printf("  UDP throughput (1472B packets): %10.2f MB/s\n", mbps);
    }
    
    udp_socket_destroy(socket);
}

// =============================================================================
// Main Benchmark Runner
// =============================================================================

int main(void) {
    printf("========================================\n");
    printf("TCP/UDP Protocol Benchmarks\n");
    printf("Based on Beej's Network Programming Guide\n");
    printf("========================================\n\n");
    
    printf("=== TCP Benchmarks ===\n");
    bench_tcp_server_create_destroy();
    bench_tcp_client_create_destroy();
    bench_tcp_send_small();
    bench_tcp_send_large();
    bench_tcp_recv();
    
    printf("\n=== UDP Benchmarks ===\n");
    bench_udp_socket_create_destroy();
    bench_udp_sendto_small();
    bench_udp_sendto_large();
    bench_udp_recvfrom();
    
    printf("\n=== Socket Options Benchmarks ===\n");
    bench_socket_set_options();
    
    printf("\n=== Address Resolution Benchmarks ===\n");
    bench_address_resolution();
    bench_address_conversion();
    
    printf("\n=== Multiplexer Benchmarks ===\n");
    bench_multiplexer_create_destroy();
    bench_multiplexer_operations();
    bench_multiplexer_wait();
    
    // Throughput benchmarks
    bench_tcp_throughput();
    bench_udp_throughput();
    
    printf("\n========================================\n");
    printf("Benchmarks completed successfully!\n");
    printf("========================================\n");
    
    return 0;
}
