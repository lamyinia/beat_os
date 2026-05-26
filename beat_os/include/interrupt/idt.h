#ifndef BEAT_OS_INTERRUPT_IDT_H
#define BEAT_OS_INTERRUPT_IDT_H

#include "drivers/log.h"

enum interrupt_vector {
    INTERRUPT_VECTOR_DIVIDE_ERROR = 0,
    INTERRUPT_VECTOR_DEBUG = 1,
    INTERRUPT_VECTOR_NMI = 2,
    INTERRUPT_VECTOR_BREAKPOINT = 3,
    INTERRUPT_VECTOR_OVERFLOW = 4,
    INTERRUPT_VECTOR_BOUND_RANGE_EXCEEDED = 5,
    INTERRUPT_VECTOR_INVALID_OPCODE = 6,
    INTERRUPT_VECTOR_DEVICE_NOT_AVAILABLE = 7,
    INTERRUPT_VECTOR_DOUBLE_FAULT = 8,
    INTERRUPT_VECTOR_INVALID_TSS = 10,
    INTERRUPT_VECTOR_SEGMENT_NOT_PRESENT = 11,
    INTERRUPT_VECTOR_STACK_SEGMENT_FAULT = 12,
    INTERRUPT_VECTOR_GENERAL_PROTECTION = 13,
    INTERRUPT_VECTOR_PAGE_FAULT = 14,
    INTERRUPT_VECTOR_X87_FLOATING_POINT = 16,
    INTERRUPT_VECTOR_ALIGNMENT_CHECK = 17,
    INTERRUPT_VECTOR_MACHINE_CHECK = 18,
    INTERRUPT_VECTOR_SIMD_FLOATING_POINT = 19,
    INTERRUPT_VECTOR_VIRTUALIZATION = 20,
};

struct interrupt_context {
    uint64_t rax;
    uint64_t rbx;
    uint64_t rcx;
    uint64_t rdx;
    uint64_t rbp;
    uint64_t rdi;
    uint64_t rsi;
    uint64_t r8;
    uint64_t r9;
    uint64_t r10;
    uint64_t r11;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
    uint64_t vector;
    uint64_t error_code;
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
};

void idt_init(void);
void interrupt_handle_exception(struct interrupt_context *context);

void interrupt_expect_exception(uint64_t vector, uint64_t resume_rip);
int interrupt_exception_was_handled(void);

void interrupt_test_trigger_divide_error(void);
void interrupt_test_divide_error_resume(void);
void interrupt_test_trigger_general_protection(void);
void interrupt_test_general_protection_resume(void);
void interrupt_test_trigger_page_fault(void);
void interrupt_test_page_fault_resume(void);

#endif
