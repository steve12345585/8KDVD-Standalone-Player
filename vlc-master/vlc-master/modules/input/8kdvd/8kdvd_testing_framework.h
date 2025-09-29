#ifndef VLC_8KDVD_TESTING_FRAMEWORK_H
#define VLC_8KDVD_TESTING_FRAMEWORK_H

#include <vlc_common.h>
#include <vlc_input.h>
#include <vlc_fourcc.h>
#include <vlc_block.h>
#include <vlc_es.h>
#include <vlc_es_out.h>
#include <vlc_input_item.h>
#include <stdint.h>
#include <stdbool.h>

// 8KDVD Testing Framework for Comprehensive Automated Testing
typedef struct kdvd_testing_framework_t kdvd_testing_framework_t;

// 8KDVD Test Types
typedef enum {
    EIGHTKDVD_TEST_UNIT = 0,
    EIGHTKDVD_TEST_INTEGRATION,
    EIGHTKDVD_TEST_PERFORMANCE,
    EIGHTKDVD_TEST_STRESS,
    EIGHTKDVD_TEST_COMPATIBILITY,
    EIGHTKDVD_TEST_SECURITY,
    EIGHTKDVD_TEST_USER_INTERFACE,
    EIGHTKDVD_TEST_REGRESSION
} kdvd_test_type_t;

// 8KDVD Test Status
typedef enum {
    EIGHTKDVD_TEST_PENDING = 0,
    EIGHTKDVD_TEST_RUNNING,
    EIGHTKDVD_TEST_PASSED,
    EIGHTKDVD_TEST_FAILED,
    EIGHTKDVD_TEST_SKIPPED,
    EIGHTKDVD_TEST_ERROR,
    EIGHTKDVD_TEST_TIMEOUT
} kdvd_test_status_t;

// 8KDVD Test Priority
typedef enum {
    EIGHTKDVD_TEST_PRIORITY_LOW = 0,
    EIGHTKDVD_TEST_PRIORITY_MEDIUM,
    EIGHTKDVD_TEST_PRIORITY_HIGH,
    EIGHTKDVD_TEST_PRIORITY_CRITICAL
} kdvd_test_priority_t;

// 8KDVD Test Result
typedef struct kdvd_test_result_t {
    char test_name[128];            // Test name
    kdvd_test_type_t test_type;     // Test type
    kdvd_test_status_t status;      // Test status
    kdvd_test_priority_t priority;   // Test priority
    uint64_t start_time;            // Test start time
    uint64_t end_time;              // Test end time
    uint64_t duration_ms;            // Test duration in milliseconds
    uint32_t assertions_passed;     // Assertions passed
    uint32_t assertions_failed;     // Assertions failed
    uint32_t errors_count;          // Error count
    uint32_t warnings_count;        // Warning count
    char error_message[512];         // Error message
    char warning_message[512];      // Warning message
    char output_log[1024];          // Output log
    void *test_data;                // Test data pointer
} kdvd_test_result_t;

// 8KDVD Test Suite
typedef struct kdvd_test_suite_t {
    char name[128];                 // Test suite name
    char description[256];          // Test suite description
    kdvd_test_type_t test_type;     // Test suite type
    kdvd_test_priority_t priority;  // Test suite priority
    uint32_t test_count;            // Test count
    kdvd_test_result_t *tests;      // Tests array
    uint32_t passed_count;          // Passed test count
    uint32_t failed_count;          // Failed test count
    uint32_t skipped_count;         // Skipped test count
    uint32_t error_count;           // Error test count
    uint64_t total_duration_ms;     // Total duration in milliseconds
    bool is_running;                // Test suite running status
    void *suite_data;               // Suite data pointer
} kdvd_test_suite_t;

// 8KDVD Testing Framework Statistics
typedef struct kdvd_testing_stats_t {
    uint64_t tests_executed;        // Total tests executed
    uint64_t tests_passed;          // Total tests passed
    uint64_t tests_failed;          // Total tests failed
    uint64_t tests_skipped;         // Total tests skipped
    uint64_t tests_error;           // Total tests with errors
    uint64_t tests_timeout;         // Total tests timed out
    uint64_t test_suites_executed;  // Total test suites executed
    uint64_t test_suites_passed;    // Total test suites passed
    uint64_t test_suites_failed;    // Total test suites failed
    uint64_t assertions_checked;    // Total assertions checked
    uint64_t assertions_passed;     // Total assertions passed
    uint64_t assertions_failed;     // Total assertions failed
    uint64_t errors_found;          // Total errors found
    uint64_t warnings_found;         // Total warnings found
    float average_test_duration;     // Average test duration
    float average_suite_duration;   // Average suite duration
    float pass_rate;                // Test pass rate percentage
    uint32_t current_test_count;     // Current test count
    uint32_t current_suite_count;    // Current suite count
    uint32_t memory_usage_mb;        // Memory usage in MB
} kdvd_testing_stats_t;

// 8KDVD Testing Framework Functions
kdvd_testing_framework_t* kdvd_testing_framework_create(vlc_object_t *obj);
void kdvd_testing_framework_destroy(kdvd_testing_framework_t *framework);

// Test Execution
int kdvd_testing_framework_run_test(kdvd_testing_framework_t *framework, const char *test_name);
int kdvd_testing_framework_run_suite(kdvd_testing_framework_t *framework, const char *suite_name);
int kdvd_testing_framework_run_all_tests(kdvd_testing_framework_t *framework);
int kdvd_testing_framework_run_tests_by_type(kdvd_testing_framework_t *framework, kdvd_test_type_t test_type);
int kdvd_testing_framework_run_tests_by_priority(kdvd_testing_framework_t *framework, kdvd_test_priority_t priority);

// Test Management
int kdvd_testing_framework_add_test(kdvd_testing_framework_t *framework, const char *test_name, kdvd_test_type_t test_type, kdvd_test_priority_t priority, int (*test_function)(void));
int kdvd_testing_framework_remove_test(kdvd_testing_framework_t *framework, const char *test_name);
int kdvd_testing_framework_add_suite(kdvd_testing_framework_t *framework, const char *suite_name, kdvd_test_type_t test_type, kdvd_test_priority_t priority);
int kdvd_testing_framework_remove_suite(kdvd_testing_framework_t *framework, const char *suite_name);

// Test Assertions
int kdvd_testing_framework_assert_true(kdvd_testing_framework_t *framework, bool condition, const char *message);
int kdvd_testing_framework_assert_false(kdvd_testing_framework_t *framework, bool condition, const char *message);
int kdvd_testing_framework_assert_equal(kdvd_testing_framework_t *framework, int32_t expected, int32_t actual, const char *message);
int kdvd_testing_framework_assert_not_equal(kdvd_testing_framework_t *framework, int32_t expected, int32_t actual, const char *message);
int kdvd_testing_framework_assert_string_equal(kdvd_testing_framework_t *framework, const char *expected, const char *actual, const char *message);
int kdvd_testing_framework_assert_string_not_equal(kdvd_testing_framework_t *framework, const char *expected, const char *actual, const char *message);
int kdvd_testing_framework_assert_null(kdvd_testing_framework_t *framework, void *pointer, const char *message);
int kdvd_testing_framework_assert_not_null(kdvd_testing_framework_t *framework, void *pointer, const char *message);

// Test Results
kdvd_test_result_t kdvd_testing_framework_get_test_result(kdvd_testing_framework_t *framework, const char *test_name);
kdvd_test_suite_t kdvd_testing_framework_get_suite_result(kdvd_testing_framework_t *framework, const char *suite_name);
int kdvd_testing_framework_get_test_count(kdvd_testing_framework_t *framework);
int kdvd_testing_framework_get_suite_count(kdvd_testing_framework_t *framework);
kdvd_test_result_t* kdvd_testing_framework_get_all_test_results(kdvd_testing_framework_t *framework);
kdvd_test_suite_t* kdvd_testing_framework_get_all_suite_results(kdvd_testing_framework_t *framework);

// Test Configuration
int kdvd_testing_framework_set_timeout(kdvd_testing_framework_t *framework, uint32_t timeout_ms);
int kdvd_testing_framework_set_parallel_tests(kdvd_testing_framework_t *framework, bool enable);
int kdvd_testing_framework_set_verbose_output(kdvd_testing_framework_t *framework, bool enable);
int kdvd_testing_framework_set_output_file(kdvd_testing_framework_t *framework, const char *output_path);

// Test Reporting
int kdvd_testing_framework_generate_report(kdvd_testing_framework_t *framework, const char *report_path);
int kdvd_testing_framework_generate_html_report(kdvd_testing_framework_t *framework, const char *html_path);
int kdvd_testing_framework_generate_xml_report(kdvd_testing_framework_t *framework, const char *xml_path);
int kdvd_testing_framework_generate_json_report(kdvd_testing_framework_t *framework, const char *json_path);

// Performance and Statistics
kdvd_testing_stats_t kdvd_testing_framework_get_stats(kdvd_testing_framework_t *framework);
int kdvd_testing_framework_reset_stats(kdvd_testing_framework_t *framework);
int kdvd_testing_framework_set_performance_mode(kdvd_testing_framework_t *framework, const char *mode);

// Memory Management
int kdvd_testing_framework_allocate_buffers(kdvd_testing_framework_t *framework);
int kdvd_testing_framework_free_buffers(kdvd_testing_framework_t *framework);
int kdvd_testing_framework_get_memory_usage(kdvd_testing_framework_t *framework, uint32_t *usage_mb);

// Error Handling
int kdvd_testing_framework_get_last_error(kdvd_testing_framework_t *framework, char *error_buffer, size_t buffer_size);
int kdvd_testing_framework_clear_errors(kdvd_testing_framework_t *framework);

// Debug and Logging
void kdvd_testing_framework_set_debug(kdvd_testing_framework_t *framework, bool enable);
void kdvd_testing_framework_log_info(kdvd_testing_framework_t *framework);
void kdvd_testing_framework_log_stats(kdvd_testing_framework_t *framework);

#endif // VLC_8KDVD_TESTING_FRAMEWORK_H
