CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c11 -I./include -pthread
LDFLAGS = -pthread

# Directories
SRC_DIR = src
BUILD_DIR = build
TEST_DIR = tests
BENCH_DIR = benchmarks
INCLUDE_DIR = include

# Source files
COMMON_SRC = $(SRC_DIR)/common/common.c
HTTP_SRC = $(SRC_DIR)/http/http_parser.c
WEBSERVER_SRC = $(SRC_DIR)/webserver/webserver.c
DATABASE_SRC = $(SRC_DIR)/database/database.c
CACHE_SRC = $(SRC_DIR)/cache/cache.c
MQUEUE_SRC = $(SRC_DIR)/mqueue/mqueue.c
DISTRIBUTED_SRC = $(SRC_DIR)/distributed/distributed.c
HTTP_STATUS_SRC = $(SRC_DIR)/http_status/http_status.c
AUTH_SRC = $(SRC_DIR)/auth/auth.c
CRYPTO_SRC = $(SRC_DIR)/crypto/crypto.c
SECURITY_SRC = $(SRC_DIR)/security/security.c
WEBSOCKET_SRC = $(SRC_DIR)/websocket/websocket.c
SQL_SRC = $(SRC_DIR)/sql/sql.c
NOSQL_SRC = $(SRC_DIR)/nosql/nosql.c
ARCHITECTURE_SRC = $(SRC_DIR)/architecture/architecture.c
SCALING_SRC = $(SRC_DIR)/scaling/scaling.c
LOGGING_SRC = $(SRC_DIR)/logging/logging.c
MONITORING_SRC = $(SRC_DIR)/monitoring/monitoring.c
TRACING_SRC = $(SRC_DIR)/tracing/tracing.c
TESTING_SRC = $(SRC_DIR)/testing/testing.c
CONTAINER_SRC = $(SRC_DIR)/container/container.c
CICD_SRC = $(SRC_DIR)/cicd/cicd.c
CLOUD_SRC = $(SRC_DIR)/cloud/cloud.c
DB_PERFORMANCE_SRC = $(SRC_DIR)/db_performance/db_performance.c
CACHE_STRATEGIES_SRC = $(SRC_DIR)/cache_strategies/cache_strategies.c
CONCURRENCY_SRC = $(SRC_DIR)/concurrency/concurrency.c
NETWORK_SERIALIZATION_SRC = $(SRC_DIR)/network_serialization/network_serialization.c
LATENCY_OBSERVABILITY_SRC = $(SRC_DIR)/latency_observability/latency_observability.c
TCP_UDP_SRC = $(SRC_DIR)/tcp_udp/tcp_udp.c

ALL_SRC = $(COMMON_SRC) $(HTTP_SRC) $(WEBSERVER_SRC) $(DATABASE_SRC) \
          $(CACHE_SRC) $(MQUEUE_SRC) $(DISTRIBUTED_SRC) $(HTTP_STATUS_SRC) \
          $(AUTH_SRC) $(CRYPTO_SRC) $(SECURITY_SRC) $(WEBSOCKET_SRC) \
          $(SQL_SRC) $(NOSQL_SRC) $(ARCHITECTURE_SRC) $(SCALING_SRC) \
          $(LOGGING_SRC) $(MONITORING_SRC) $(TRACING_SRC) $(TESTING_SRC) \
          $(CONTAINER_SRC) $(CICD_SRC) $(CLOUD_SRC) $(DB_PERFORMANCE_SRC) \
          $(CACHE_STRATEGIES_SRC) $(CONCURRENCY_SRC) $(NETWORK_SERIALIZATION_SRC) \
          $(LATENCY_OBSERVABILITY_SRC) $(TCP_UDP_SRC)

# Object files
COMMON_OBJ = $(BUILD_DIR)/common.o
HTTP_OBJ = $(BUILD_DIR)/http_parser.o
WEBSERVER_OBJ = $(BUILD_DIR)/webserver.o
DATABASE_OBJ = $(BUILD_DIR)/database.o
CACHE_OBJ = $(BUILD_DIR)/cache.o
MQUEUE_OBJ = $(BUILD_DIR)/mqueue.o
DISTRIBUTED_OBJ = $(BUILD_DIR)/distributed.o
HTTP_STATUS_OBJ = $(BUILD_DIR)/http_status.o
AUTH_OBJ = $(BUILD_DIR)/auth.o
CRYPTO_OBJ = $(BUILD_DIR)/crypto.o
SECURITY_OBJ = $(BUILD_DIR)/security.o
WEBSOCKET_OBJ = $(BUILD_DIR)/websocket.o
SQL_OBJ = $(BUILD_DIR)/sql.o
NOSQL_OBJ = $(BUILD_DIR)/nosql.o
ARCHITECTURE_OBJ = $(BUILD_DIR)/architecture.o
SCALING_OBJ = $(BUILD_DIR)/scaling.o
LOGGING_OBJ = $(BUILD_DIR)/logging.o
MONITORING_OBJ = $(BUILD_DIR)/monitoring.o
TRACING_OBJ = $(BUILD_DIR)/tracing.o
TESTING_OBJ = $(BUILD_DIR)/testing.o
CONTAINER_OBJ = $(BUILD_DIR)/container.o
CICD_OBJ = $(BUILD_DIR)/cicd.o
CLOUD_OBJ = $(BUILD_DIR)/cloud.o
DB_PERFORMANCE_OBJ = $(BUILD_DIR)/db_performance.o
CACHE_STRATEGIES_OBJ = $(BUILD_DIR)/cache_strategies.o
CONCURRENCY_OBJ = $(BUILD_DIR)/concurrency.o
NETWORK_SERIALIZATION_OBJ = $(BUILD_DIR)/network_serialization.o
LATENCY_OBSERVABILITY_OBJ = $(BUILD_DIR)/latency_observability.o
TCP_UDP_OBJ = $(BUILD_DIR)/tcp_udp.o

ALL_OBJ = $(COMMON_OBJ) $(HTTP_OBJ) $(WEBSERVER_OBJ) $(DATABASE_OBJ) \
          $(CACHE_OBJ) $(MQUEUE_OBJ) $(DISTRIBUTED_OBJ) $(HTTP_STATUS_OBJ) \
          $(AUTH_OBJ) $(CRYPTO_OBJ) $(SECURITY_OBJ) $(WEBSOCKET_OBJ) \
          $(SQL_OBJ) $(NOSQL_OBJ) $(ARCHITECTURE_OBJ) $(SCALING_OBJ) \
          $(LOGGING_OBJ) $(MONITORING_OBJ) $(TRACING_OBJ) $(TESTING_OBJ) \
          $(CONTAINER_OBJ) $(CICD_OBJ) $(CLOUD_OBJ) $(DB_PERFORMANCE_OBJ) \
          $(CACHE_STRATEGIES_OBJ) $(CONCURRENCY_OBJ) $(NETWORK_SERIALIZATION_OBJ) \
          $(LATENCY_OBSERVABILITY_OBJ) $(TCP_UDP_OBJ)

# Test executables
TEST_HTTP = $(BUILD_DIR)/test_http
TEST_WEBSERVER = $(BUILD_DIR)/test_webserver
TEST_DATABASE = $(BUILD_DIR)/test_database
TEST_CACHE = $(BUILD_DIR)/test_cache
TEST_MQUEUE = $(BUILD_DIR)/test_mqueue
TEST_DISTRIBUTED = $(BUILD_DIR)/test_distributed
TEST_HTTP_STATUS = $(BUILD_DIR)/test_http_status
TEST_AUTH = $(BUILD_DIR)/test_auth
TEST_CRYPTO = $(BUILD_DIR)/test_crypto
TEST_DB_PERFORMANCE = $(BUILD_DIR)/test_db_performance
TEST_CACHE_STRATEGIES = $(BUILD_DIR)/test_cache_strategies
TEST_CONCURRENCY = $(BUILD_DIR)/test_concurrency
TEST_NETWORK_SERIALIZATION = $(BUILD_DIR)/test_network_serialization
TEST_LATENCY_OBSERVABILITY = $(BUILD_DIR)/test_latency_observability
TEST_TCP_UDP = $(BUILD_DIR)/test_tcp_udp

ALL_TESTS = $(TEST_DB_PERFORMANCE) $(TEST_CACHE_STRATEGIES) $(TEST_CONCURRENCY) \
            $(TEST_NETWORK_SERIALIZATION) $(TEST_LATENCY_OBSERVABILITY) $(TEST_TCP_UDP)

# Benchmark executables
BENCH_HTTP = $(BUILD_DIR)/bench_http
BENCH_DATABASE = $(BUILD_DIR)/bench_database
BENCH_CACHE = $(BUILD_DIR)/bench_cache
BENCH_MQUEUE = $(BUILD_DIR)/bench_mqueue
BENCH_HTTP_STATUS = $(BUILD_DIR)/bench_http_status
BENCH_AUTH = $(BUILD_DIR)/bench_auth
BENCH_CRYPTO = $(BUILD_DIR)/bench_crypto
BENCH_DB_PERFORMANCE = $(BUILD_DIR)/bench_db_performance
BENCH_CACHE_STRATEGIES = $(BUILD_DIR)/bench_cache_strategies
BENCH_CONCURRENCY = $(BUILD_DIR)/bench_concurrency
BENCH_NETWORK_SERIALIZATION = $(BUILD_DIR)/bench_network_serialization
BENCH_LATENCY_OBSERVABILITY = $(BUILD_DIR)/bench_latency_observability
BENCH_TCP_UDP = $(BUILD_DIR)/bench_tcp_udp

ALL_BENCHMARKS = $(BENCH_DB_PERFORMANCE) $(BENCH_CACHE_STRATEGIES) $(BENCH_CONCURRENCY) \
                 $(BENCH_NETWORK_SERIALIZATION) $(BENCH_LATENCY_OBSERVABILITY) $(BENCH_TCP_UDP)

.PHONY: all clean test benchmark

all: $(BUILD_DIR) $(ALL_OBJ) $(ALL_TESTS) $(ALL_BENCHMARKS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Compile object files
$(COMMON_OBJ): $(COMMON_SRC) $(INCLUDE_DIR)/common.h
	$(CC) $(CFLAGS) -c $< -o $@

$(HTTP_OBJ): $(HTTP_SRC) $(INCLUDE_DIR)/http_parser.h $(INCLUDE_DIR)/common.h
	$(CC) $(CFLAGS) -c $< -o $@

$(WEBSERVER_OBJ): $(WEBSERVER_SRC) $(INCLUDE_DIR)/webserver.h $(INCLUDE_DIR)/http_parser.h $(INCLUDE_DIR)/common.h
	$(CC) $(CFLAGS) -c $< -o $@

$(DATABASE_OBJ): $(DATABASE_SRC) $(INCLUDE_DIR)/database.h $(INCLUDE_DIR)/common.h
	$(CC) $(CFLAGS) -c $< -o $@

$(CACHE_OBJ): $(CACHE_SRC) $(INCLUDE_DIR)/cache.h $(INCLUDE_DIR)/common.h
	$(CC) $(CFLAGS) -c $< -o $@

$(MQUEUE_OBJ): $(MQUEUE_SRC) $(INCLUDE_DIR)/mqueue.h $(INCLUDE_DIR)/common.h
	$(CC) $(CFLAGS) -c $< -o $@

$(DISTRIBUTED_OBJ): $(DISTRIBUTED_SRC) $(INCLUDE_DIR)/distributed.h $(INCLUDE_DIR)/common.h
	$(CC) $(CFLAGS) -c $< -o $@

# New modules
$(HTTP_STATUS_OBJ): $(HTTP_STATUS_SRC) $(INCLUDE_DIR)/http_status.h $(INCLUDE_DIR)/common.h
	$(CC) $(CFLAGS) -c $< -o $@

$(AUTH_OBJ): $(AUTH_SRC) $(INCLUDE_DIR)/auth.h $(INCLUDE_DIR)/common.h
	$(CC) $(CFLAGS) -c $< -o $@

$(CRYPTO_OBJ): $(CRYPTO_SRC) $(INCLUDE_DIR)/crypto.h $(INCLUDE_DIR)/common.h
	$(CC) $(CFLAGS) -c $< -o $@

$(SECURITY_OBJ): $(SECURITY_SRC) $(INCLUDE_DIR)/security.h $(INCLUDE_DIR)/common.h
	$(CC) $(CFLAGS) -c $< -o $@

$(WEBSOCKET_OBJ): $(WEBSOCKET_SRC) $(INCLUDE_DIR)/websocket.h $(INCLUDE_DIR)/common.h
	$(CC) $(CFLAGS) -c $< -o $@

$(SQL_OBJ): $(SQL_SRC) $(INCLUDE_DIR)/sql.h $(INCLUDE_DIR)/common.h
	$(CC) $(CFLAGS) -c $< -o $@

$(NOSQL_OBJ): $(NOSQL_SRC) $(INCLUDE_DIR)/nosql.h $(INCLUDE_DIR)/common.h
	$(CC) $(CFLAGS) -c $< -o $@

$(ARCHITECTURE_OBJ): $(ARCHITECTURE_SRC) $(INCLUDE_DIR)/architecture.h $(INCLUDE_DIR)/common.h
	$(CC) $(CFLAGS) -c $< -o $@

$(SCALING_OBJ): $(SCALING_SRC) $(INCLUDE_DIR)/scaling.h $(INCLUDE_DIR)/common.h
	$(CC) $(CFLAGS) -c $< -o $@

$(LOGGING_OBJ): $(LOGGING_SRC) $(INCLUDE_DIR)/logging.h $(INCLUDE_DIR)/common.h
	$(CC) $(CFLAGS) -c $< -o $@

$(MONITORING_OBJ): $(MONITORING_SRC) $(INCLUDE_DIR)/monitoring.h $(INCLUDE_DIR)/common.h
	$(CC) $(CFLAGS) -c $< -o $@

$(TRACING_OBJ): $(TRACING_SRC) $(INCLUDE_DIR)/tracing.h $(INCLUDE_DIR)/common.h
	$(CC) $(CFLAGS) -c $< -o $@

$(TESTING_OBJ): $(TESTING_SRC) $(INCLUDE_DIR)/testing.h $(INCLUDE_DIR)/common.h
	$(CC) $(CFLAGS) -c $< -o $@

$(CONTAINER_OBJ): $(CONTAINER_SRC) $(INCLUDE_DIR)/container.h $(INCLUDE_DIR)/common.h
	$(CC) $(CFLAGS) -c $< -o $@

$(CICD_OBJ): $(CICD_SRC) $(INCLUDE_DIR)/cicd.h $(INCLUDE_DIR)/common.h
	$(CC) $(CFLAGS) -c $< -o $@

$(CLOUD_OBJ): $(CLOUD_SRC) $(INCLUDE_DIR)/cloud.h $(INCLUDE_DIR)/common.h
	$(CC) $(CFLAGS) -c $< -o $@

# New performance optimization modules
$(DB_PERFORMANCE_OBJ): $(DB_PERFORMANCE_SRC) $(INCLUDE_DIR)/db_performance.h $(INCLUDE_DIR)/common.h
	$(CC) $(CFLAGS) -c $< -o $@

$(CACHE_STRATEGIES_OBJ): $(CACHE_STRATEGIES_SRC) $(INCLUDE_DIR)/cache_strategies.h $(INCLUDE_DIR)/common.h
	$(CC) $(CFLAGS) -c $< -o $@

$(CONCURRENCY_OBJ): $(CONCURRENCY_SRC) $(INCLUDE_DIR)/concurrency.h $(INCLUDE_DIR)/common.h
	$(CC) $(CFLAGS) -c $< -o $@

$(NETWORK_SERIALIZATION_OBJ): $(NETWORK_SERIALIZATION_SRC) $(INCLUDE_DIR)/network_serialization.h $(INCLUDE_DIR)/common.h
	$(CC) $(CFLAGS) -c $< -o $@

$(LATENCY_OBSERVABILITY_OBJ): $(LATENCY_OBSERVABILITY_SRC) $(INCLUDE_DIR)/latency_observability.h $(INCLUDE_DIR)/common.h
	$(CC) $(CFLAGS) -c $< -o $@

$(TCP_UDP_OBJ): $(TCP_UDP_SRC) $(INCLUDE_DIR)/tcp_udp.h $(INCLUDE_DIR)/common.h
	$(CC) $(CFLAGS) -c $< -o $@

# Build tests - Performance optimization modules
$(TEST_DB_PERFORMANCE): $(TEST_DIR)/test_db_performance.c $(COMMON_OBJ) $(DB_PERFORMANCE_OBJ)
	$(CC) $(CFLAGS) $< $(COMMON_OBJ) $(DB_PERFORMANCE_OBJ) -o $@ $(LDFLAGS)

$(TEST_CACHE_STRATEGIES): $(TEST_DIR)/test_cache_strategies.c $(COMMON_OBJ) $(CACHE_STRATEGIES_OBJ)
	$(CC) $(CFLAGS) $< $(COMMON_OBJ) $(CACHE_STRATEGIES_OBJ) -o $@ $(LDFLAGS)

$(TEST_CONCURRENCY): $(TEST_DIR)/test_concurrency.c $(COMMON_OBJ) $(CONCURRENCY_OBJ)
	$(CC) $(CFLAGS) $< $(COMMON_OBJ) $(CONCURRENCY_OBJ) -o $@ $(LDFLAGS)

$(TEST_NETWORK_SERIALIZATION): $(TEST_DIR)/test_network_serialization.c $(COMMON_OBJ) $(NETWORK_SERIALIZATION_OBJ)
	$(CC) $(CFLAGS) $< $(COMMON_OBJ) $(NETWORK_SERIALIZATION_OBJ) -o $@ $(LDFLAGS)

$(TEST_LATENCY_OBSERVABILITY): $(TEST_DIR)/test_latency_observability.c $(COMMON_OBJ) $(LATENCY_OBSERVABILITY_OBJ)
	$(CC) $(CFLAGS) $< $(COMMON_OBJ) $(LATENCY_OBSERVABILITY_OBJ) -o $@ $(LDFLAGS)

$(TEST_TCP_UDP): $(TEST_DIR)/test_tcp_udp.c $(COMMON_OBJ) $(TCP_UDP_OBJ)
	$(CC) $(CFLAGS) $< $(COMMON_OBJ) $(TCP_UDP_OBJ) -o $@ $(LDFLAGS)

# Build benchmarks - Performance optimization modules
$(BENCH_DB_PERFORMANCE): $(BENCH_DIR)/bench_db_performance.c $(COMMON_OBJ) $(DB_PERFORMANCE_OBJ)
	$(CC) $(CFLAGS) $< $(COMMON_OBJ) $(DB_PERFORMANCE_OBJ) -o $@ $(LDFLAGS)

$(BENCH_CACHE_STRATEGIES): $(BENCH_DIR)/bench_cache_strategies.c $(COMMON_OBJ) $(CACHE_STRATEGIES_OBJ)
	$(CC) $(CFLAGS) $< $(COMMON_OBJ) $(CACHE_STRATEGIES_OBJ) -o $@ $(LDFLAGS)

$(BENCH_CONCURRENCY): $(BENCH_DIR)/bench_concurrency.c $(COMMON_OBJ) $(CONCURRENCY_OBJ)
	$(CC) $(CFLAGS) $< $(COMMON_OBJ) $(CONCURRENCY_OBJ) -o $@ $(LDFLAGS)

$(BENCH_NETWORK_SERIALIZATION): $(BENCH_DIR)/bench_network_serialization.c $(COMMON_OBJ) $(NETWORK_SERIALIZATION_OBJ)
	$(CC) $(CFLAGS) $< $(COMMON_OBJ) $(NETWORK_SERIALIZATION_OBJ) -o $@ $(LDFLAGS)

$(BENCH_LATENCY_OBSERVABILITY): $(BENCH_DIR)/bench_latency_observability.c $(COMMON_OBJ) $(LATENCY_OBSERVABILITY_OBJ)
	$(CC) $(CFLAGS) $< $(COMMON_OBJ) $(LATENCY_OBSERVABILITY_OBJ) -o $@ $(LDFLAGS)

$(BENCH_TCP_UDP): $(BENCH_DIR)/bench_tcp_udp.c $(COMMON_OBJ) $(TCP_UDP_OBJ)
	$(CC) $(CFLAGS) $< $(COMMON_OBJ) $(TCP_UDP_OBJ) -o $@ $(LDFLAGS)

# Run tests
test: $(ALL_TESTS)
	@echo "Running tests..."
	@for test in $(ALL_TESTS); do \
		echo "Running $$test..."; \
		$$test || exit 1; \
	done
	@echo "All tests passed!"

# Run benchmarks
benchmark: $(ALL_BENCHMARKS)
	@echo "Running benchmarks..."
	@for bench in $(ALL_BENCHMARKS); do \
		echo "Running $$bench..."; \
		$$bench; \
	done

clean:
	rm -rf $(BUILD_DIR)
