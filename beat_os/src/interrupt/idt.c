#include "interrupt/idt.h"
#include "lib/string.h"

struct idt_entry {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t ist;
    uint8_t type_attributes;
    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t reserved;
} __attribute__((packed));

struct idt_pointer {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

extern void isr_divide_error(void);
extern void isr_debug(void);
extern void isr_nmi(void);
extern void isr_breakpoint(void);
extern void isr_overflow(void);
extern void isr_bound_range_exceeded(void);
extern void isr_invalid_opcode(void);
extern void isr_device_not_available(void);
extern void isr_double_fault(void);
extern void isr_invalid_tss(void);
extern void isr_segment_not_present(void);
extern void isr_stack_segment_fault(void);
extern void isr_general_protection(void);
extern void isr_page_fault(void);
extern void isr_x87_floating_point(void);
extern void isr_alignment_check(void);
extern void isr_machine_check(void);
extern void isr_simd_floating_point(void);
extern void isr_virtualization(void);

struct interrupt_descriptor {
    uint8_t vector;
    void (*handler)(void);
};

static const struct interrupt_descriptor interrupt_descriptors[] = {
    { INTERRUPT_VECTOR_DIVIDE_ERROR, isr_divide_error },
    { INTERRUPT_VECTOR_DEBUG, isr_debug },
    { INTERRUPT_VECTOR_NMI, isr_nmi },
    { INTERRUPT_VECTOR_BREAKPOINT, isr_breakpoint },
    { INTERRUPT_VECTOR_OVERFLOW, isr_overflow },
    { INTERRUPT_VECTOR_BOUND_RANGE_EXCEEDED, isr_bound_range_exceeded },
    { INTERRUPT_VECTOR_INVALID_OPCODE, isr_invalid_opcode },
    { INTERRUPT_VECTOR_DEVICE_NOT_AVAILABLE, isr_device_not_available },
    { INTERRUPT_VECTOR_DOUBLE_FAULT, isr_double_fault },
    { INTERRUPT_VECTOR_INVALID_TSS, isr_invalid_tss },
    { INTERRUPT_VECTOR_SEGMENT_NOT_PRESENT, isr_segment_not_present },
    { INTERRUPT_VECTOR_STACK_SEGMENT_FAULT, isr_stack_segment_fault },
    { INTERRUPT_VECTOR_GENERAL_PROTECTION, isr_general_protection },
    { INTERRUPT_VECTOR_PAGE_FAULT, isr_page_fault },
    { INTERRUPT_VECTOR_X87_FLOATING_POINT, isr_x87_floating_point },
    { INTERRUPT_VECTOR_ALIGNMENT_CHECK, isr_alignment_check },
    { INTERRUPT_VECTOR_MACHINE_CHECK, isr_machine_check },
    { INTERRUPT_VECTOR_SIMD_FLOATING_POINT, isr_simd_floating_point },
    { INTERRUPT_VECTOR_VIRTUALIZATION, isr_virtualization },
};

static struct idt_entry idt[256];
static struct {
    int active;
    int handled;
    uint64_t vector;
    uint64_t resume_rip;
} expected_exception;

static inline void lidt(const struct idt_pointer *pointer) {
    __asm__ volatile ("lidt %0" : : "m"(*pointer));
}

static inline uint64_t read_cr2(void) {
    uint64_t value;
    __asm__ volatile ("mov %%cr2, %0" : "=r"(value));
    return value;
}

static inline void halt_forever(void) {
    for (;;) {
        __asm__ volatile ("cli; hlt");
    }
}

static void idt_set_gate(uint8_t vector, void (*handler)(void)) {
    uint64_t address = (uint64_t)handler;

    idt[vector].offset_low = (uint16_t)(address & 0xFFFF);
    idt[vector].selector = 0x08;
    idt[vector].ist = 0;
    idt[vector].type_attributes = 0x8E;
    idt[vector].offset_mid = (uint16_t)((address >> 16) & 0xFFFF);
    idt[vector].offset_high = (uint32_t)((address >> 32) & 0xFFFFFFFF);
    idt[vector].reserved = 0;
}

static const char *interrupt_name(uint64_t vector) {
    switch (vector) {
    case INTERRUPT_VECTOR_DIVIDE_ERROR:
        return "#DE divide error";
    case INTERRUPT_VECTOR_DEBUG:
        return "#DB debug";
    case INTERRUPT_VECTOR_NMI:
        return "NMI non-maskable interrupt";
    case INTERRUPT_VECTOR_BREAKPOINT:
        return "#BP breakpoint";
    case INTERRUPT_VECTOR_OVERFLOW:
        return "#OF overflow";
    case INTERRUPT_VECTOR_BOUND_RANGE_EXCEEDED:
        return "#BR bound range exceeded";
    case INTERRUPT_VECTOR_INVALID_OPCODE:
        return "#UD invalid opcode";
    case INTERRUPT_VECTOR_DEVICE_NOT_AVAILABLE:
        return "#NM device not available";
    case INTERRUPT_VECTOR_DOUBLE_FAULT:
        return "#DF double fault";
    case INTERRUPT_VECTOR_INVALID_TSS:
        return "#TS invalid TSS";
    case INTERRUPT_VECTOR_SEGMENT_NOT_PRESENT:
        return "#NP segment not present";
    case INTERRUPT_VECTOR_STACK_SEGMENT_FAULT:
        return "#SS stack-segment fault";
    case INTERRUPT_VECTOR_GENERAL_PROTECTION:
        return "#GP general protection fault";
    case INTERRUPT_VECTOR_PAGE_FAULT:
        return "#PF page fault";
    case INTERRUPT_VECTOR_X87_FLOATING_POINT:
        return "#MF x87 floating-point exception";
    case INTERRUPT_VECTOR_ALIGNMENT_CHECK:
        return "#AC alignment check";
    case INTERRUPT_VECTOR_MACHINE_CHECK:
        return "#MC machine check";
    case INTERRUPT_VECTOR_SIMD_FLOATING_POINT:
        return "#XM SIMD floating-point exception";
    case INTERRUPT_VECTOR_VIRTUALIZATION:
        return "#VE virtualization exception";
    default:
        return "unknown exception";
    }
}

static int interrupt_is_expected(uint64_t vector) {
    return expected_exception.active && expected_exception.vector == vector;
}

static const char *interrupt_prefix(int expected) {
    if (expected) {
        return "[TEST] interrupt: ";
    }
    return "[PANIC] interrupt: ";
}

static void write_line(const char *prefix, const char *text) {
    console_write(prefix);
    console_write(text);
    console_write("\n");
}

static void write_field_hex64(const char *prefix,
                              const char *label,
                              uint64_t value) {
    console_write(prefix);
    console_write(label);
    console_write("=");
    console_write_hex64(value);
    console_write("\n");
}

static void write_register_pair(const char *prefix,
                                const char *left_label,
                                uint64_t left_value,
                                const char *right_label,
                                uint64_t right_value) {
    console_write(prefix);
    console_write(left_label);
    console_write("=");
    console_write_hex64(left_value);
    console_write(" ");
    console_write(right_label);
    console_write("=");
    console_write_hex64(right_value);
    console_write("\n");
}

static void write_exception_summary(const char *prefix,
                                    const struct interrupt_context *context) {
    console_write(prefix);
    console_write("name=");
    console_write(interrupt_name(context->vector));
    console_write("\n");
    write_field_hex64(prefix, "vector", context->vector);
    write_field_hex64(prefix, "error_code", context->error_code);
    write_field_hex64(prefix, "rip", context->rip);
    write_field_hex64(prefix, "cs", context->cs);
    write_field_hex64(prefix, "rflags", context->rflags);

    if (context->vector == INTERRUPT_VECTOR_PAGE_FAULT) {
        write_field_hex64(prefix, "fault_address", read_cr2());
    }
}

static void write_general_registers(const char *prefix,
                                    const struct interrupt_context *context) {
    write_line(prefix, "register dump");
    write_register_pair(prefix, "rax", context->rax, "rbx", context->rbx);
    write_register_pair(prefix, "rcx", context->rcx, "rdx", context->rdx);
    write_register_pair(prefix, "rsi", context->rsi, "rdi", context->rdi);
    write_register_pair(prefix, "rbp", context->rbp, "r8", context->r8);
    write_register_pair(prefix, "r9", context->r9, "r10", context->r10);
    write_register_pair(prefix, "r11", context->r11, "r12", context->r12);
    write_register_pair(prefix, "r13", context->r13, "r14", context->r14);
    write_field_hex64(prefix, "r15", context->r15);
    console_write("\n");
}

void idt_init(void) {
    struct idt_pointer pointer;
    uint64_t i;

    memset(idt, 0, sizeof(idt));

    for (i = 0; i < sizeof(interrupt_descriptors) / sizeof(interrupt_descriptors[0]); i++) {
        idt_set_gate(interrupt_descriptors[i].vector,
                     interrupt_descriptors[i].handler);
    }

    pointer.limit = (uint16_t)(sizeof(idt) - 1);
    pointer.base = (uint64_t)idt;

    lidt(&pointer);
}

void interrupt_expect_exception(uint64_t vector, uint64_t resume_rip) {
    expected_exception.active = 1;
    expected_exception.handled = 0;
    expected_exception.vector = vector;
    expected_exception.resume_rip = resume_rip;
}

int interrupt_exception_was_handled(void) {
    return expected_exception.handled;
}

void interrupt_handle_exception(struct interrupt_context *context) {
    int expected = interrupt_is_expected(context->vector);
    const char *prefix = interrupt_prefix(expected);

    if (expected) {
        log_line(LOG_INFO, "interrupt: expected exception trapped");
    } else {
        log_line(LOG_PANIC, "interrupt: fatal exception trapped");
    }

    write_exception_summary(prefix, context);
    write_general_registers(prefix, context);

    if (expected) {
        log_line(LOG_INFO, "interrupt: expected exception handled");
        expected_exception.active = 0;
        expected_exception.handled = 1;
        context->rip = expected_exception.resume_rip;
        return;
    }

    log_line(LOG_PANIC, "interrupt: halting after fatal exception");
    halt_forever();
}
