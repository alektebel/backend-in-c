# TCP/IP and UDP Implementation Guidelines

This document provides implementation guidelines for the TCP/IP and UDP protocol templates, inspired by Beej's Network Programming Guide in C.

## Overview

The `tcp_udp.h` module provides a comprehensive abstraction layer for network socket programming in C. It includes both TCP (connection-oriented) and UDP (connectionless) protocols, along with utilities for socket configuration, address resolution, and I/O multiplexing.

## Core Concepts from Beej's Guide

### 1. TCP vs UDP

**TCP (Transmission Control Protocol)**
- Connection-oriented: requires establishing a connection before data transfer
- Reliable: guarantees delivery and order of packets
- Stream-based: data is read as a continuous stream
- Use cases: HTTP, FTP, SSH, email protocols

**UDP (User Datagram Protocol)**
- Connectionless: no connection establishment needed
- Unreliable: no guarantee of delivery or order
- Message-based: data is sent and received in discrete datagrams
- Use cases: DNS, DHCP, streaming media, gaming

### 2. Socket Lifecycle

**TCP Server Lifecycle:**
```
socket() -> bind() -> listen() -> accept() -> recv()/send() -> close()
```

**TCP Client Lifecycle:**
```
socket() -> connect() -> send()/recv() -> close()
```

**UDP Socket Lifecycle:**
```
socket() -> bind() -> recvfrom()/sendto() -> close()
```

## Implementation Guidelines

### TCP Server Implementation

```c
// Create server
tcp_server_t* server = tcp_server_create("0.0.0.0", "8080", 10);

// Start listening
tcp_server_listen(server);

// Accept connections in a loop
while (running) {
    tcp_connection_t* client = NULL;
    if (tcp_server_accept(server, &client) == SUCCESS) {
        // Handle client connection
        handle_client(client);
        tcp_close(client);
        free(client);
    }
}

// Cleanup
tcp_server_destroy(server);
```

**Key Points:**
- Use "0.0.0.0" to bind to all interfaces
- Use "127.0.0.1" to bind only to localhost
- Backlog parameter determines the queue size for pending connections
- Always check return values for errors

### TCP Client Implementation

```c
// Create client
tcp_client_t* client = tcp_client_create("example.com", 8080);

// Connect to server
if (tcp_client_connect(client) == SUCCESS) {
    // Send data
    const char* message = "Hello, Server!";
    ssize_t sent = tcp_send(client->connection, message, strlen(message));
    
    // Receive response
    char buffer[1024];
    ssize_t received = tcp_recv(client->connection, buffer, sizeof(buffer));
    
    if (received > 0) {
        buffer[received] = '\0';
        printf("Received: %s\n", buffer);
    }
}

// Cleanup
tcp_client_destroy(client);
```

**Key Points:**
- Always null-terminate received strings if treating them as C strings
- Check return values: negative = error, 0 = connection closed, positive = bytes transferred
- Handle partial sends/receives in real implementations

### UDP Server Implementation

```c
// Create UDP socket
udp_socket_t* socket = udp_socket_create(SOCKET_FAMILY_IPV4);

// Bind to port
udp_bind(socket, "0.0.0.0", 7070);

// Receive datagrams
while (running) {
    char buffer[1500];  // MTU size for ethernet
    char src_addr[256];
    int src_port;
    
    ssize_t received = udp_recvfrom(socket, buffer, sizeof(buffer),
                                    src_addr, sizeof(src_addr), &src_port);
    
    if (received > 0) {
        // Process datagram and send response
        udp_sendto(socket, buffer, received, src_addr, src_port);
    }
}

// Cleanup
udp_socket_destroy(socket);
```

**Key Points:**
- UDP datagrams can arrive out of order
- Maximum UDP packet size is typically 65,507 bytes (theoretical), but use 1472 for IPv4 to avoid fragmentation
- No delivery guarantee - implement acknowledgments if needed

### UDP Client Implementation

```c
// Create UDP socket
udp_socket_t* socket = udp_socket_create(SOCKET_FAMILY_IPV4);

// Send datagram
const char* message = "Hello, UDP Server!";
ssize_t sent = udp_sendto(socket, message, strlen(message),
                         "127.0.0.1", 7070);

// Receive response (with timeout recommended)
char buffer[1500];
char src_addr[256];
int src_port;

ssize_t received = udp_recvfrom(socket, buffer, sizeof(buffer),
                                src_addr, sizeof(src_addr), &src_port);

// Cleanup
udp_socket_destroy(socket);
```

### Connected UDP Sockets

UDP sockets can be "connected" to simplify send/recv operations:

```c
udp_socket_t* socket = udp_socket_create(SOCKET_FAMILY_IPV4);

// Connect to specific endpoint
udp_connect(socket, "127.0.0.1", 7070);

// Now use simpler send/recv instead of sendto/recvfrom
udp_send(socket, data, len);
udp_recv(socket, buffer, sizeof(buffer));

udp_socket_destroy(socket);
```

**Note:** This doesn't establish a connection like TCP - it just filters incoming packets and allows simplified API usage.

## Socket Options (Chapter 7.2 in Beej's Guide)

### Common Socket Options

**SO_REUSEADDR** - Allow reuse of local addresses
```c
socket_set_reuseaddr(sockfd, true);
```
Use this to avoid "Address already in use" errors when restarting servers.

**SO_KEEPALIVE** - Keep connections alive with periodic probes
```c
socket_set_keepalive(sockfd, true);
```
Detects dead connections but with long timeouts (typically 2+ hours).

**TCP_NODELAY** - Disable Nagle's algorithm
```c
socket_set_nodelay(sockfd, true);
```
Reduces latency by sending small packets immediately instead of buffering them.

**Non-blocking I/O**
```c
socket_set_nonblocking(sockfd, true);
```
Operations return immediately even if they would block. Essential for multiplexing.

**Socket Timeouts**
```c
socket_set_recv_timeout(sockfd, 5000);  // 5 seconds
socket_set_send_timeout(sockfd, 5000);
```
Prevents indefinite blocking on send/recv operations.

## Address Resolution (Chapter 5.1 in Beej's Guide)

### Resolving Hostnames

```c
address_info_t* results = NULL;
resolve_address("www.example.com", "http", &results, SOCK_STREAM);

// Iterate through results
address_info_t* current = results;
while (current) {
    char addr_str[256];
    address_to_string(&current->address, addr_str, sizeof(addr_str));
    printf("Address: %s\n", addr_str);
    current = current->next;
}

free_address_info(results);
```

**Key Points:**
- DNS resolution can return multiple addresses (IPv4 and IPv6)
- Try each address until one succeeds
- Use NULL for hostname to get addresses suitable for binding (server)

## I/O Multiplexing (Chapter 7.3 in Beej's Guide)

### Why Multiplexing?

Multiplexing allows handling multiple connections simultaneously in a single thread:
- **select()** - Original POSIX method, works everywhere but limited to 1024 file descriptors
- **poll()** - Similar to select but no FD limit
- **epoll()** - Linux-specific, very efficient for large numbers of connections
- **kqueue()** - BSD/macOS equivalent of epoll

### Using the Multiplexer

```c
socket_multiplexer_t* mux = multiplexer_create(MULTIPLEXER_EPOLL);

// Add server socket for read events
multiplexer_add(mux, server_sockfd, EVENT_READ, NULL);

socket_event_t events[MAX_EVENTS];

while (running) {
    int num_events = multiplexer_wait(mux, events, MAX_EVENTS, 1000);
    
    for (int i = 0; i < num_events; i++) {
        if (events[i].sockfd == server_sockfd) {
            // New connection
            accept_connection();
        } else if (events[i].events & EVENT_READ) {
            // Data available to read
            handle_read(events[i].sockfd);
        } else if (events[i].events & EVENT_WRITE) {
            // Socket ready for writing
            handle_write(events[i].sockfd);
        }
    }
}

multiplexer_destroy(mux);
```

## Error Handling

### Checking Return Values

Always check return values:
```c
ssize_t sent = tcp_send(conn, data, len);
if (sent < 0) {
    int error = socket_get_last_error();
    fprintf(stderr, "Send failed: %s\n", socket_error_string(error));
    // Handle error
}
```

### Common Errors

- **EINTR** - Interrupted system call, retry the operation
- **EAGAIN/EWOULDBLOCK** - Operation would block (non-blocking sockets)
- **ECONNRESET** - Connection reset by peer
- **EPIPE** - Broken pipe (writing to closed socket)
- **ETIMEDOUT** - Connection timed out

## Performance Considerations

### TCP Performance

1. **Nagle's Algorithm** - Delays small packets to reduce network overhead
   - Disable with TCP_NODELAY for low-latency applications
   - Keep enabled for bulk data transfer

2. **Send/Receive Buffers** - Adjust buffer sizes for high-throughput
   ```c
   socket_set_sndbuf(sockfd, 256 * 1024);  // 256KB send buffer
   socket_set_rcvbuf(sockfd, 256 * 1024);  // 256KB receive buffer
   ```

3. **Connection Pooling** - Reuse connections instead of creating new ones

### UDP Performance

1. **Packet Size** - Use 1472 bytes for IPv4 to avoid fragmentation
2. **Batching** - Send multiple small messages in one datagram when possible
3. **Local Buffering** - Use larger receive buffers for high packet rates

### Concurrency Models

1. **Thread per Connection** - Simple but doesn't scale
2. **Thread Pool** - Better resource management
3. **Event-Driven I/O** - Best scalability (using multiplexing)
4. **Hybrid** - Thread pool with event-driven I/O

## Testing Guidelines

### Unit Testing

Test individual functions with mock data:
```c
// Test TCP send
tcp_connection_t conn;
const char* data = "test";
ssize_t sent = tcp_send(&conn, data, strlen(data));
assert(sent == strlen(data));
```

### Integration Testing

Test actual network communication:
- Start server in background
- Connect client and exchange data
- Verify data integrity
- Test error conditions (timeout, connection refused)

### Performance Testing

Benchmark key operations:
- Connection establishment time
- Throughput (MB/s)
- Latency (microseconds)
- Concurrent connection handling

## Implementation Roadmap

When implementing actual functionality:

1. **Phase 1: Basic TCP**
   - Socket creation and destruction
   - Connect/bind/listen/accept
   - Send/recv with blocking I/O

2. **Phase 2: Basic UDP**
   - Socket creation
   - Sendto/recvfrom
   - Connected UDP sockets

3. **Phase 3: Socket Options**
   - Implement common socket options
   - Timeouts
   - Non-blocking I/O

4. **Phase 4: Address Resolution**
   - Integrate getaddrinfo/getnameinfo
   - IPv4 and IPv6 support

5. **Phase 5: Multiplexing**
   - Implement select/poll
   - Add epoll (Linux) or kqueue (BSD/macOS)
   - Edge-triggered vs level-triggered modes

6. **Phase 6: Advanced Features**
   - Zero-copy operations (sendfile)
   - Scatter/gather I/O (readv/writev)
   - Socket authentication

## References

- **Beej's Guide to Network Programming**: https://beej.us/guide/bgnet/
- **POSIX Socket API**: IEEE Std 1003.1
- **TCP RFC**: RFC 793
- **UDP RFC**: RFC 768
- **Linux Socket Programming**: `man 2 socket`, `man 7 socket`

## Best Practices

1. **Always check return values** - Network operations can fail
2. **Handle partial sends/receives** - TCP send/recv may not transfer all data at once
3. **Use timeouts** - Prevent indefinite blocking
4. **Close sockets properly** - Avoid resource leaks
5. **Handle signals** - SIGPIPE can terminate your process
6. **Validate input** - Never trust network data
7. **Use multiplexing** - For handling multiple connections efficiently
8. **Test error paths** - Network errors are common in production
9. **Monitor resources** - Track open file descriptors and memory usage
10. **Log appropriately** - Debug network issues with good logging

## Security Considerations

1. **Input Validation** - Always validate and sanitize network input
2. **Buffer Overflows** - Use bounded string operations (strncpy, snprintf)
3. **Resource Limits** - Limit number of connections, rate limiting
4. **Encryption** - Use TLS/SSL for sensitive data (not included in this module)
5. **Authentication** - Verify client identity when needed
6. **Denial of Service** - Implement timeouts and connection limits

## Conclusion

This TCP/UDP module provides the foundation for network programming in C. For production use, you should:

1. Implement full error handling
2. Add comprehensive logging
3. Implement connection pooling
4. Add SSL/TLS support for secure communication
5. Implement rate limiting and resource management
6. Add metrics and monitoring
7. Write extensive tests including failure scenarios

The template provides the structure and API - the implementation requires careful attention to error handling, performance, and security.
