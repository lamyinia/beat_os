bits 64

section .text
global isr_divide_error
global isr_debug
global isr_nmi
global isr_breakpoint
global isr_overflow
global isr_bound_range_exceeded
global isr_invalid_opcode
global isr_device_not_available
global isr_double_fault
global isr_invalid_tss
global isr_segment_not_present
global isr_stack_segment_fault
global isr_general_protection
global isr_page_fault
global isr_x87_floating_point
global isr_alignment_check
global isr_machine_check
global isr_simd_floating_point
global isr_virtualization
global interrupt_test_trigger_divide_error
global interrupt_test_divide_error_resume
global interrupt_test_trigger_general_protection
global interrupt_test_general_protection_resume
global interrupt_test_trigger_page_fault
global interrupt_test_page_fault_resume

extern interrupt_handle_exception

%macro PUSH_CONTEXT 0
    push r15
    push r14
    push r13
    push r12
    push r11
    push r10
    push r9
    push r8
    push rsi
    push rdi
    push rbp
    push rdx
    push rcx
    push rbx
    push rax
%endmacro

%macro POP_CONTEXT 0
    pop rax
    pop rbx
    pop rcx
    pop rdx
    pop rbp
    pop rdi
    pop rsi
    pop r8
    pop r9
    pop r10
    pop r11
    pop r12
    pop r13
    pop r14
    pop r15
%endmacro

%macro ISR_NO_ERROR 2
%1:
    push 0
    push %2
    jmp isr_common
%endmacro

%macro ISR_WITH_ERROR 2
%1:
    push %2
    jmp isr_common
%endmacro

ISR_NO_ERROR isr_divide_error, 0
ISR_NO_ERROR isr_debug, 1
ISR_NO_ERROR isr_nmi, 2
ISR_NO_ERROR isr_breakpoint, 3
ISR_NO_ERROR isr_overflow, 4
ISR_NO_ERROR isr_bound_range_exceeded, 5
ISR_NO_ERROR isr_invalid_opcode, 6
ISR_NO_ERROR isr_device_not_available, 7
ISR_WITH_ERROR isr_double_fault, 8
ISR_WITH_ERROR isr_invalid_tss, 10
ISR_WITH_ERROR isr_segment_not_present, 11
ISR_WITH_ERROR isr_stack_segment_fault, 12
ISR_WITH_ERROR isr_general_protection, 13
ISR_WITH_ERROR isr_page_fault, 14
ISR_NO_ERROR isr_x87_floating_point, 16
ISR_WITH_ERROR isr_alignment_check, 17
ISR_NO_ERROR isr_machine_check, 18
ISR_NO_ERROR isr_simd_floating_point, 19
ISR_NO_ERROR isr_virtualization, 20

isr_common:
    PUSH_CONTEXT
    mov rdi, rsp
    mov rbx, rsp
    and rsp, -16
    call interrupt_handle_exception
    mov rsp, rbx
    POP_CONTEXT
    add rsp, 16
    iretq

interrupt_test_trigger_divide_error:
    xor rdx, rdx
    xor rcx, rcx
    mov rax, 1
    div rcx
interrupt_test_divide_error_resume:
    ret

interrupt_test_trigger_general_protection:
    mov rax, 0x0000800000000000
    mov al, [rax]
interrupt_test_general_protection_resume:
    ret

interrupt_test_trigger_page_fault:
    mov rax, 0x0000000040000000
    mov al, [rax]
interrupt_test_page_fault_resume:
    ret
