#include "interrupt/idt.h"
#include "test/test.h"
#include "test/test_interrupt.h"

static void test_divide_error(void) {
    interrupt_expect_exception(
        INTERRUPT_VECTOR_DIVIDE_ERROR,
        (uint64_t)interrupt_test_divide_error_resume);
    interrupt_test_trigger_divide_error();

    test_expect_true(interrupt_exception_was_handled(),
                     "divide error hits the IDT handler");
}

static void test_general_protection(void) {
    interrupt_expect_exception(
        INTERRUPT_VECTOR_GENERAL_PROTECTION,
        (uint64_t)interrupt_test_general_protection_resume);
    interrupt_test_trigger_general_protection();

    test_expect_true(interrupt_exception_was_handled(),
                     "general protection hits the IDT handler");
}

static void test_page_fault(void) {
    interrupt_expect_exception(
        INTERRUPT_VECTOR_PAGE_FAULT,
        (uint64_t)interrupt_test_page_fault_resume);
    interrupt_test_trigger_page_fault();

    test_expect_true(interrupt_exception_was_handled(),
                     "page fault hits the IDT handler");
}

void run_interrupt_tests(void) {
    test_log_section("interrupt");

    test_divide_error();
    test_general_protection();
    test_page_fault();
}
