# backend-in-c

A comprehensive collection of backend system implementations in C, including HTTP parsing, web server, database/key-value store, cache system, message queue, and distributed system components.

## Components

### 1. HTTP Parser
- HTTP request/response parsing
- Support for HTTP/1.0, HTTP/1.1, HTTP/2.0
- Header parsing and manipulation
- Query string parsing
- Request body handling

### 2. Web Server
- Multi-threaded HTTP server
- Request routing and handling
- Configurable handlers
- Connection management

### 3. Database / Key-Value Store
- In-memory hash table storage engine
- Write-Ahead Log (WAL) for durability
- Transaction support with ACID properties
- Multiple isolation levels
- Indexing support (B-tree, Hash)
- Compaction and recovery
- Concurrency control with read-write locks

### 4. Cache System (Redis-like)
- LRU (Least Recently Used) eviction policy
- LFU (Least Frequently Used) eviction policy
- TTL (Time To Live) support
- Memory-efficient storage
- Thread-safe operations
- Hit/miss statistics

### 5. Message Queue / Broker
- FIFO and priority-based ordering
- Producer-consumer pattern
- Multiple delivery guarantees (at-most-once, at-least-once, exactly-once)
- Retry mechanism
- Topic-based subscriptions
- Acknowledgment support

### 6. Distributed System Components
- Service discovery and registry
- Authentication with JWT-style tokens
- Rate limiting (token bucket)
- Circuit breaker pattern
- Retry policies with exponential backoff
- Observability (metrics collection)
- Service heartbeat monitoring

## Building

### Requirements
- GCC compiler
- POSIX-compliant system (Linux, macOS)
- pthread library
- make

### Build all components
```bash
make all
```

### Build specific components
```bash
make build/test_http
make build/bench_database
```

### Clean build artifacts
```bash
make clean
```

## Testing

Run all tests:
```bash
make test
```

Individual tests are located in the `tests/` directory:
- `test_http` - HTTP parser tests
- `test_webserver` - Web server tests
- `test_database` - Database tests
- `test_cache` - Cache system tests
- `test_mqueue` - Message queue tests
- `test_distributed` - Distributed system tests

## Benchmarking

Run all benchmarks:
```bash
make benchmark
```

Individual benchmarks are located in the `benchmarks/` directory:
- `bench_http` - HTTP parsing performance
- `bench_database` - Database operations performance
- `bench_cache` - Cache operations and eviction performance
- `bench_mqueue` - Message queue throughput

## Architecture

```
backend-in-c/
├── include/          # Header files
│   ├── common.h
│   ├── http_parser.h
│   ├── webserver.h
│   ├── database.h
│   ├── cache.h
│   ├── mqueue.h
│   └── distributed.h
├── src/              # Implementation files
│   ├── common/
│   ├── http/
│   ├── webserver/
│   ├── database/
│   ├── cache/
│   ├── mqueue/
│   └── distributed/
├── tests/            # Unit tests
├── benchmarks/       # Performance benchmarks
├── build/            # Build artifacts
└── Makefile         # Build configuration
```

## Usage Examples

### HTTP Parser
```c
#include "http_parser.h"

const char* raw_request = "GET /api/users HTTP/1.1\r\nHost: example.com\r\n\r\n";
http_request_t* request = http_request_create();
http_request_parse(request, raw_request, strlen(raw_request));

// Access parsed data
printf("Method: %s\n", http_method_to_string(request->method));
printf("Path: %s\n", request->path);

http_request_destroy(request);
```

### Database
```c
#include "database.h"

database_t* db = database_create("/tmp/mydb");
database_open(db);

// Put/Get operations
database_put(db, "user:123", "John Doe", 9);
void* value = NULL;
database_get(db, "user:123", &value, NULL);

// Transactions
transaction_t* txn = database_begin_transaction(db, ISOLATION_READ_COMMITTED);
transaction_put(txn, "key", "value", 6);
transaction_commit(txn);

database_close(db);
database_destroy(db);
```

### Cache
```c
#include "cache.h"

cache_t* cache = cache_create(1000, EVICTION_LRU);

// Put with TTL
cache_put_with_ttl(cache, "session:abc", "user_data", 10, 3600000);

// Get
void* value = NULL;
if (cache_get(cache, "session:abc", &value, NULL) == SUCCESS) {
    // Use value
    free(value);
}

cache_destroy(cache);
```

### Message Queue
```c
#include "mqueue.h"

void message_handler(const message_t* msg, void* user_data) {
    printf("Received: %s\n", (char*)msg->payload);
}

mqueue_t* queue = mqueue_create("my_queue", ORDERING_FIFO);

// Subscribe
consumer_t* consumer = mqueue_subscribe(queue, "events", message_handler, NULL);

// Publish
mqueue_publish(queue, "events", "Hello, World!", 14);

mqueue_unsubscribe(consumer);
mqueue_destroy(queue);
```

### Distributed System
```c
#include "distributed.h"

// Service registry
service_registry_t* registry = service_registry_create();
service_register(registry, "api-service", "localhost", 8080);
service_info_t* info = service_discover(registry, "api-service");

// Rate limiting
rate_limiter_t* limiter = rate_limiter_create(100, 1000);
if (rate_limiter_allow(limiter, "client1")) {
    // Process request
}

// Circuit breaker
circuit_breaker_t* breaker = circuit_breaker_create(5, 60000);
circuit_breaker_call(breaker, my_function, arg);
```

## Features

### Correctness
- Comprehensive unit tests for all components
- Memory leak detection and prevention
- Thread-safe operations
- ACID transaction support in database
- Proper error handling

### Performance
- Lock-free operations where possible
- Efficient memory management
- Optimized data structures (hash tables, LRU lists)
- Write-ahead logging for durability
- Benchmark suite for performance testing

### Scalability
- Multi-threaded web server
- Concurrent cache operations
- Parallel message processing
- Service discovery for distributed systems
- Rate limiting and circuit breakers

## License

MIT License

## Contributing

Contributions are welcome! Please ensure:
1. Code follows existing style
2. All tests pass
3. New features include tests
4. Benchmarks are updated if performance-critical