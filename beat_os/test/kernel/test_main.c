#include "drivers/log.h"
#include "test/test.h"
#include "test/test_interrupt.h"
#include "test/test_string.h"

static int tests_run = 0;
static int tests_failed = 0;

static void test_pass(const char *name) {
    console_write("[PASS] ");
    console_write(name);
    console_write("\n");
}

static void test_fail(const char *name) {
    console_write("[FAIL] ");
    console_write(name);
    console_write("\n");
    tests_failed++;
}

void test_expect_true(int condition, const char *name) {
    tests_run++;
    if (condition) {
        test_pass(name);
    } else {
        test_fail(name);
    }
}

void test_log_section(const char *name) {
    console_write("[TEST] section: ");
    console_write(name);
    console_write("\n");
}

static void test_boot_sanity(void) {
    test_expect_true(1, "boot sanity");
}

static void test_math_sanity(void) {
    test_expect_true((2 + 2) == 4, "math sanity");
}

static void test_console_smoke(void) {
    tests_run++;
    console_write("[TEST] console smoke emitted\n");
    test_pass("console smoke");
}

int run_tests(void) {
    tests_run = 0;
    tests_failed = 0;

    log_line(LOG_INFO, "test: starting kernel test run");

    test_boot_sanity();
    test_math_sanity();
    test_console_smoke();
    run_string_tests();
    run_interrupt_tests();

    console_write("[TEST] summary: run=");
    console_write_hex64((uint64_t)tests_run);
    console_write(" failed=");
    console_write_hex64((uint64_t)tests_failed);
    console_write("\n");

    if (tests_failed == 0) {
        log_line(LOG_INFO, "test: all tests passed");
    } else {
        log_line(LOG_PANIC, "test: failures detected");
    }

    return tests_failed;
}
