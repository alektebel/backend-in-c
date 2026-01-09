#ifndef TESTING_H
#define TESTING_H

#include "common.h"

// ============================================================================
// Unit Testing Framework
// ============================================================================

typedef struct test_suite test_suite_t;
typedef struct test_case test_case_t;
typedef struct test_result test_result_t;

typedef void (*test_func_t)(test_case_t* test);
typedef void (*setup_func_t)(void* context);
typedef void (*teardown_func_t)(void* context);

// Test suite operations
test_suite_t* test_suite_create(const char* name);
void test_suite_destroy(test_suite_t* suite);
int test_suite_add_test(test_suite_t* suite, const char* test_name, test_func_t func);
int test_suite_set_setup(test_suite_t* suite, setup_func_t setup);
int test_suite_set_teardown(test_suite_t* suite, teardown_func_t teardown);
int test_suite_set_context(test_suite_t* suite, void* context);
test_result_t* test_suite_run(test_suite_t* suite);

// Test assertions
void test_assert(test_case_t* test, int condition, const char* message);
void test_assert_equal_int(test_case_t* test, int64_t expected, int64_t actual);
void test_assert_equal_double(test_case_t* test, double expected, double actual, 
                             double epsilon);
void test_assert_equal_string(test_case_t* test, const char* expected, 
                             const char* actual);
void test_assert_null(test_case_t* test, const void* ptr);
void test_assert_not_null(test_case_t* test, const void* ptr);
void test_assert_true(test_case_t* test, int condition);
void test_assert_false(test_case_t* test, int condition);

// Test result operations
int test_result_get_total_count(const test_result_t* result);
int test_result_get_passed_count(const test_result_t* result);
int test_result_get_failed_count(const test_result_t* result);
int test_result_get_skipped_count(const test_result_t* result);
void test_result_print(const test_result_t* result);
char* test_result_to_xml(const test_result_t* result);  // JUnit XML format
void test_result_destroy(test_result_t* result);

// ============================================================================
// Mocking Framework
// ============================================================================

typedef struct mock_object mock_object_t;
typedef struct mock_expectation mock_expectation_t;

mock_object_t* mock_create(const char* name);
void mock_destroy(mock_object_t* mock);
int mock_expect_call(mock_object_t* mock, const char* method_name);
int mock_expect_call_with_args(mock_object_t* mock, const char* method_name,
                               const void** args, size_t arg_count);
int mock_set_return_value(mock_object_t* mock, const char* method_name,
                          const void* return_value);
int mock_verify(mock_object_t* mock);
void mock_reset(mock_object_t* mock);

// ============================================================================
// Integration Testing
// ============================================================================

typedef struct integration_test integration_test_t;
typedef struct test_environment test_environment_t;

// Test environment (for setting up databases, services, etc.)
test_environment_t* test_environment_create(void);
void test_environment_destroy(test_environment_t* env);
int test_environment_start_service(test_environment_t* env, const char* service_name,
                                  const char* config);
int test_environment_stop_service(test_environment_t* env, const char* service_name);
int test_environment_reset_database(test_environment_t* env);
int test_environment_seed_data(test_environment_t* env, const char* data_file);

// Integration test operations
integration_test_t* integration_test_create(const char* name,
                                           test_environment_t* env);
void integration_test_destroy(integration_test_t* test);
int integration_test_run(integration_test_t* test);

// ============================================================================
// Load Testing (k6/JMeter-style)
// ============================================================================

typedef struct load_test load_test_t;
typedef struct load_test_config load_test_config_t;
typedef struct load_test_result load_test_result_t;

struct load_test_config {
    int virtual_users;
    uint64_t duration_seconds;
    uint64_t ramp_up_seconds;
    int requests_per_second;  // Rate limiting
    const char* target_url;
};

struct load_test_result {
    uint64_t total_requests;
    uint64_t successful_requests;
    uint64_t failed_requests;
    double avg_response_time_ms;
    double min_response_time_ms;
    double max_response_time_ms;
    double p50_response_time_ms;
    double p95_response_time_ms;
    double p99_response_time_ms;
    double requests_per_second;
    double data_transferred_mb;
};

// Load test operations
load_test_t* load_test_create(const load_test_config_t* config);
void load_test_destroy(load_test_t* test);
int load_test_add_scenario(load_test_t* test, const char* name,
                          int (*scenario_func)(void*), void* user_data);
load_test_result_t* load_test_run(load_test_t* test);
void load_test_result_print(const load_test_result_t* result);
char* load_test_result_to_json(const load_test_result_t* result);
void load_test_result_destroy(load_test_result_t* result);

// ============================================================================
// Stress Testing
// ============================================================================

typedef struct stress_test stress_test_t;

stress_test_t* stress_test_create(void);
void stress_test_destroy(stress_test_t* test);
int stress_test_set_target(stress_test_t* test, const char* target_url);
int stress_test_set_max_users(stress_test_t* test, int max_users);
int stress_test_set_ramp_up_rate(stress_test_t* test, int users_per_second);
load_test_result_t* stress_test_run(stress_test_t* test);

// ============================================================================
// Performance Benchmarking
// ============================================================================

typedef struct benchmark benchmark_t;
typedef struct benchmark_result benchmark_result_t;

benchmark_t* benchmark_create(const char* name);
void benchmark_destroy(benchmark_t* bench);
int benchmark_add_case(benchmark_t* bench, const char* case_name,
                      void (*func)(void*), void* user_data);
benchmark_result_t* benchmark_run(benchmark_t* bench, int iterations);

struct benchmark_result {
    const char* name;
    int iterations;
    uint64_t total_time_ns;
    uint64_t min_time_ns;
    uint64_t max_time_ns;
    double avg_time_ns;
    double operations_per_second;
};

void benchmark_result_print(const benchmark_result_t* result);
char* benchmark_result_to_json(const benchmark_result_t* result);
void benchmark_result_destroy(benchmark_result_t* result);

// ============================================================================
// Test Coverage
// ============================================================================

typedef struct coverage_analyzer coverage_analyzer_t;
typedef struct coverage_report coverage_report_t;

coverage_analyzer_t* coverage_analyzer_create(void);
void coverage_analyzer_destroy(coverage_analyzer_t* analyzer);
int coverage_analyzer_start(coverage_analyzer_t* analyzer);
int coverage_analyzer_stop(coverage_analyzer_t* analyzer);
coverage_report_t* coverage_analyzer_generate_report(coverage_analyzer_t* analyzer);

struct coverage_report {
    double line_coverage_percent;
    double branch_coverage_percent;
    double function_coverage_percent;
    int lines_covered;
    int lines_total;
    int branches_covered;
    int branches_total;
};

void coverage_report_print(const coverage_report_t* report);
char* coverage_report_to_html(const coverage_report_t* report);
void coverage_report_destroy(coverage_report_t* report);

// ============================================================================
// Fuzz Testing
// ============================================================================

typedef struct fuzzer fuzzer_t;

fuzzer_t* fuzzer_create(void);
void fuzzer_destroy(fuzzer_t* fuzzer);
int fuzzer_set_target(fuzzer_t* fuzzer, int (*target_func)(const void*, size_t));
int fuzzer_set_seed(fuzzer_t* fuzzer, uint64_t seed);
int fuzzer_set_max_iterations(fuzzer_t* fuzzer, int iterations);
int fuzzer_run(fuzzer_t* fuzzer);

// ============================================================================
// Test Data Generation
// ============================================================================

typedef struct test_data_generator test_data_generator_t;

test_data_generator_t* test_data_generator_create(void);
void test_data_generator_destroy(test_data_generator_t* generator);
char* test_data_generate_string(test_data_generator_t* gen, int min_len, int max_len);
int test_data_generate_int(test_data_generator_t* gen, int min, int max);
double test_data_generate_double(test_data_generator_t* gen, double min, double max);
char* test_data_generate_email(test_data_generator_t* gen);
char* test_data_generate_url(test_data_generator_t* gen);
char* test_data_generate_uuid(test_data_generator_t* gen);

#endif // TESTING_H
