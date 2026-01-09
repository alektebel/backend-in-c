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

ALL_SRC = $(COMMON_SRC) $(HTTP_SRC) $(WEBSERVER_SRC) $(DATABASE_SRC) \
          $(CACHE_SRC) $(MQUEUE_SRC) $(DISTRIBUTED_SRC)

# Object files
COMMON_OBJ = $(BUILD_DIR)/common.o
HTTP_OBJ = $(BUILD_DIR)/http_parser.o
WEBSERVER_OBJ = $(BUILD_DIR)/webserver.o
DATABASE_OBJ = $(BUILD_DIR)/database.o
CACHE_OBJ = $(BUILD_DIR)/cache.o
MQUEUE_OBJ = $(BUILD_DIR)/mqueue.o
DISTRIBUTED_OBJ = $(BUILD_DIR)/distributed.o

ALL_OBJ = $(COMMON_OBJ) $(HTTP_OBJ) $(WEBSERVER_OBJ) $(DATABASE_OBJ) \
          $(CACHE_OBJ) $(MQUEUE_OBJ) $(DISTRIBUTED_OBJ)

# Test executables
TEST_HTTP = $(BUILD_DIR)/test_http
TEST_WEBSERVER = $(BUILD_DIR)/test_webserver
TEST_DATABASE = $(BUILD_DIR)/test_database
TEST_CACHE = $(BUILD_DIR)/test_cache
TEST_MQUEUE = $(BUILD_DIR)/test_mqueue
TEST_DISTRIBUTED = $(BUILD_DIR)/test_distributed

ALL_TESTS = $(TEST_HTTP) $(TEST_WEBSERVER) $(TEST_DATABASE) $(TEST_CACHE) \
            $(TEST_MQUEUE) $(TEST_DISTRIBUTED)

# Benchmark executables
BENCH_HTTP = $(BUILD_DIR)/bench_http
BENCH_DATABASE = $(BUILD_DIR)/bench_database
BENCH_CACHE = $(BUILD_DIR)/bench_cache
BENCH_MQUEUE = $(BUILD_DIR)/bench_mqueue

ALL_BENCHMARKS = $(BENCH_HTTP) $(BENCH_DATABASE) $(BENCH_CACHE) $(BENCH_MQUEUE)

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

# Build benchmarks
$(BENCH_HTTP): $(BENCH_DIR)/bench_http.c $(COMMON_OBJ) $(HTTP_OBJ)
	$(CC) $(CFLAGS) $< $(COMMON_OBJ) $(HTTP_OBJ) -o $@ $(LDFLAGS)

$(BENCH_DATABASE): $(BENCH_DIR)/bench_database.c $(COMMON_OBJ) $(DATABASE_OBJ)
	$(CC) $(CFLAGS) $< $(COMMON_OBJ) $(DATABASE_OBJ) -o $@ $(LDFLAGS)

$(BENCH_CACHE): $(BENCH_DIR)/bench_cache.c $(COMMON_OBJ) $(CACHE_OBJ)
	$(CC) $(CFLAGS) $< $(COMMON_OBJ) $(CACHE_OBJ) -o $@ $(LDFLAGS)

$(BENCH_MQUEUE): $(BENCH_DIR)/bench_mqueue.c $(COMMON_OBJ) $(MQUEUE_OBJ)
	$(CC) $(CFLAGS) $< $(COMMON_OBJ) $(MQUEUE_OBJ) -o $@ $(LDFLAGS)

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
