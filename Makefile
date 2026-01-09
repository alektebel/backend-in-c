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

ALL_SRC = $(COMMON_SRC) $(HTTP_SRC) $(WEBSERVER_SRC) $(DATABASE_SRC) \
          $(CACHE_SRC) $(MQUEUE_SRC) $(DISTRIBUTED_SRC) $(HTTP_STATUS_SRC) \
          $(AUTH_SRC) $(CRYPTO_SRC) $(SECURITY_SRC) $(WEBSOCKET_SRC) \
          $(SQL_SRC) $(NOSQL_SRC) $(ARCHITECTURE_SRC) $(SCALING_SRC) \
          $(LOGGING_SRC) $(MONITORING_SRC) $(TRACING_SRC) $(TESTING_SRC) \
          $(CONTAINER_SRC) $(CICD_SRC) $(CLOUD_SRC)

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

ALL_OBJ = $(COMMON_OBJ) $(HTTP_OBJ) $(WEBSERVER_OBJ) $(DATABASE_OBJ) \
          $(CACHE_OBJ) $(MQUEUE_OBJ) $(DISTRIBUTED_OBJ) $(HTTP_STATUS_OBJ) \
          $(AUTH_OBJ) $(CRYPTO_OBJ) $(SECURITY_OBJ) $(WEBSOCKET_OBJ) \
          $(SQL_OBJ) $(NOSQL_OBJ) $(ARCHITECTURE_OBJ) $(SCALING_OBJ) \
          $(LOGGING_OBJ) $(MONITORING_OBJ) $(TRACING_OBJ) $(TESTING_OBJ) \
          $(CONTAINER_OBJ) $(CICD_OBJ) $(CLOUD_OBJ)

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
TEST_SECURITY = $(BUILD_DIR)/test_security

ALL_TESTS = $(TEST_HTTP) $(TEST_WEBSERVER) $(TEST_DATABASE) $(TEST_CACHE) \
            $(TEST_MQUEUE) $(TEST_DISTRIBUTED) $(TEST_HTTP_STATUS) $(TEST_AUTH) \
            $(TEST_CRYPTO) $(TEST_SECURITY)

# Benchmark executables
BENCH_HTTP = $(BUILD_DIR)/bench_http
BENCH_DATABASE = $(BUILD_DIR)/bench_database
BENCH_CACHE = $(BUILD_DIR)/bench_cache
BENCH_MQUEUE = $(BUILD_DIR)/bench_mqueue
BENCH_HTTP_STATUS = $(BUILD_DIR)/bench_http_status
BENCH_AUTH = $(BUILD_DIR)/bench_auth
BENCH_CRYPTO = $(BUILD_DIR)/bench_crypto
BENCH_SECURITY = $(BUILD_DIR)/bench_security

ALL_BENCHMARKS = $(BENCH_HTTP) $(BENCH_DATABASE) $(BENCH_CACHE) $(BENCH_MQUEUE) \
                 $(BENCH_HTTP_STATUS) $(BENCH_AUTH) $(BENCH_CRYPTO) $(BENCH_SECURITY)

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

# Build tests
$(TEST_HTTP): $(TEST_DIR)/test_http.c $(COMMON_OBJ) $(HTTP_OBJ)
	$(CC) $(CFLAGS) $< $(COMMON_OBJ) $(HTTP_OBJ) -o $@ $(LDFLAGS)

$(TEST_WEBSERVER): $(TEST_DIR)/test_webserver.c $(COMMON_OBJ) $(HTTP_OBJ) $(WEBSERVER_OBJ)
	$(CC) $(CFLAGS) $< $(COMMON_OBJ) $(HTTP_OBJ) $(WEBSERVER_OBJ) -o $@ $(LDFLAGS)

$(TEST_DATABASE): $(TEST_DIR)/test_database.c $(COMMON_OBJ) $(DATABASE_OBJ)
	$(CC) $(CFLAGS) $< $(COMMON_OBJ) $(DATABASE_OBJ) -o $@ $(LDFLAGS)

$(TEST_CACHE): $(TEST_DIR)/test_cache.c $(COMMON_OBJ) $(CACHE_OBJ)
	$(CC) $(CFLAGS) $< $(COMMON_OBJ) $(CACHE_OBJ) -o $@ $(LDFLAGS)

$(TEST_MQUEUE): $(TEST_DIR)/test_mqueue.c $(COMMON_OBJ) $(MQUEUE_OBJ)
	$(CC) $(CFLAGS) $< $(COMMON_OBJ) $(MQUEUE_OBJ) -o $@ $(LDFLAGS)

$(TEST_DISTRIBUTED): $(TEST_DIR)/test_distributed.c $(COMMON_OBJ) $(DISTRIBUTED_OBJ)
	$(CC) $(CFLAGS) $< $(COMMON_OBJ) $(DISTRIBUTED_OBJ) -o $@ $(LDFLAGS)

# New module tests
$(TEST_HTTP_STATUS): $(TEST_DIR)/test_http_status.c $(COMMON_OBJ) $(HTTP_STATUS_OBJ)
	$(CC) $(CFLAGS) $< $(COMMON_OBJ) $(HTTP_STATUS_OBJ) -o $@ $(LDFLAGS)

$(TEST_AUTH): $(TEST_DIR)/test_auth.c $(COMMON_OBJ) $(AUTH_OBJ)
	$(CC) $(CFLAGS) $< $(COMMON_OBJ) $(AUTH_OBJ) -o $@ $(LDFLAGS)

$(TEST_CRYPTO): $(TEST_DIR)/test_crypto.c $(COMMON_OBJ) $(CRYPTO_OBJ)
	$(CC) $(CFLAGS) $< $(COMMON_OBJ) $(CRYPTO_OBJ) -o $@ $(LDFLAGS)

$(TEST_SECURITY): $(TEST_DIR)/test_security.c $(COMMON_OBJ) $(SECURITY_OBJ)
	$(CC) $(CFLAGS) $< $(COMMON_OBJ) $(SECURITY_OBJ) -o $@ $(LDFLAGS)

# Build benchmarks
$(BENCH_HTTP): $(BENCH_DIR)/bench_http.c $(COMMON_OBJ) $(HTTP_OBJ)
	$(CC) $(CFLAGS) $< $(COMMON_OBJ) $(HTTP_OBJ) -o $@ $(LDFLAGS)

$(BENCH_DATABASE): $(BENCH_DIR)/bench_database.c $(COMMON_OBJ) $(DATABASE_OBJ)
	$(CC) $(CFLAGS) $< $(COMMON_OBJ) $(DATABASE_OBJ) -o $@ $(LDFLAGS)

$(BENCH_CACHE): $(BENCH_DIR)/bench_cache.c $(COMMON_OBJ) $(CACHE_OBJ)
	$(CC) $(CFLAGS) $< $(COMMON_OBJ) $(CACHE_OBJ) -o $@ $(LDFLAGS)

$(BENCH_MQUEUE): $(BENCH_DIR)/bench_mqueue.c $(COMMON_OBJ) $(MQUEUE_OBJ)
	$(CC) $(CFLAGS) $< $(COMMON_OBJ) $(MQUEUE_OBJ) -o $@ $(LDFLAGS)

# New module benchmarks
$(BENCH_HTTP_STATUS): $(BENCH_DIR)/bench_http_status.c $(COMMON_OBJ) $(HTTP_STATUS_OBJ)
	$(CC) $(CFLAGS) $< $(COMMON_OBJ) $(HTTP_STATUS_OBJ) -o $@ $(LDFLAGS)

$(BENCH_AUTH): $(BENCH_DIR)/bench_auth.c $(COMMON_OBJ) $(AUTH_OBJ)
	$(CC) $(CFLAGS) $< $(COMMON_OBJ) $(AUTH_OBJ) -o $@ $(LDFLAGS)

$(BENCH_CRYPTO): $(BENCH_DIR)/bench_crypto.c $(COMMON_OBJ) $(CRYPTO_OBJ)
	$(CC) $(CFLAGS) $< $(COMMON_OBJ) $(CRYPTO_OBJ) -o $@ $(LDFLAGS)

$(BENCH_SECURITY): $(BENCH_DIR)/bench_security.c $(COMMON_OBJ) $(SECURITY_OBJ)
	$(CC) $(CFLAGS) $< $(COMMON_OBJ) $(SECURITY_OBJ) -o $@ $(LDFLAGS)

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
