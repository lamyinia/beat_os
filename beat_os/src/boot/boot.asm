; boot.asm - Multiboot2 header + 32→64 bit mode transition
; GRUB loads kernel in 32-bit protected mode at 0x100000
; We must: set up page tables -> enable long mode -> jump to 64-bit code

%define COM1_PORT 0x3F8

; ==================== Multiboot2 Header ====================
section .multiboot2 align=8
multiboot2_start:
    dd 0xE85250D6
    dd 0
    dd multiboot2_end - multiboot2_start
    dd -(0xE85250D6 + 0 + (multiboot2_end - multiboot2_start))

    dw 0
    dw 0
    dd 8
multiboot2_end:

; ==================== Page Tables (BSS) ====================
section .bss align=4096
p4_table:
    resb 4096
p3_table:
    resb 4096
p2_table:
    resb 4096

multiboot_magic_value:
    resd 1
multiboot_info_ptr:
    resd 1

stack_bottom:
    resb 16384
stack_top:

; ==================== Early Boot Messages ====================
section .rodata
boot_msg_start:        db "[BOOT] 32-bit entry", 10, 0
boot_msg_tables:       db "[BOOT] page tables ready", 10, 0
boot_msg_paging:       db "[BOOT] paging + long mode enabled", 10, 0
boot_msg_long_mode:    db "[BOOT] entered 64-bit long mode", 10, 0

; ==================== 32-bit Entry Point ====================
section .text
bits 32
global _start
extern kernel_main

_start:
    cli
    mov [multiboot_magic_value], eax
    mov [multiboot_info_ptr], ebx
    mov esp, stack_top

    call serial_init_32
    mov esi, boot_msg_start
    call serial_write_32

    call set_up_page_tables
    mov esi, boot_msg_tables
    call serial_write_32

    call enable_paging
    mov esi, boot_msg_paging
    call serial_write_32

    lgdt [gdt64.pointer]
    jmp gdt64.code:long_mode_entry

serial_init_32:
    mov dx, COM1_PORT + 1
    mov al, 0x00
    out dx, al
    mov dx, COM1_PORT + 3
    mov al, 0x80
    out dx, al
    mov dx, COM1_PORT + 0
    mov al, 0x03
    out dx, al
    mov dx, COM1_PORT + 1
    mov al, 0x00
    out dx, al
    mov dx, COM1_PORT + 3
    mov al, 0x03
    out dx, al
    mov dx, COM1_PORT + 2
    mov al, 0xC7
    out dx, al
    mov dx, COM1_PORT + 4
    mov al, 0x0B
    out dx, al
    ret

serial_putchar_32:
    mov dx, COM1_PORT + 5
.wait:
    in al, dx
    test al, 0x20
    jz .wait
    mov dx, COM1_PORT
    mov al, bl
    out dx, al
    ret

serial_write_32:
.next:
    lodsb
    test al, al
    jz .done
    cmp al, 10
    jne .emit
    mov bl, 13
    call serial_putchar_32
.emit:
    mov bl, al
    call serial_putchar_32
    jmp .next
.done:
    ret

set_up_page_tables:
    mov eax, p3_table
    or eax, 0b11
    mov [p4_table], eax

    mov eax, p2_table
    or eax, 0b11
    mov [p3_table], eax

    mov ecx, 0
.map_p2:
    mov eax, 0x200000
    mul ecx
    or eax, 0b10000011
    mov [p2_table + ecx * 8], eax

    inc ecx
    cmp ecx, 512
    jne .map_p2

    ret

enable_paging:
    mov eax, p4_table
    mov cr3, eax

    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax

    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8
    wrmsr

    mov eax, cr0
    or eax, 1 << 31
    mov cr0, eax

    ret

; ==================== 64-bit GDT ====================
section .rodata align=16
gdt64:
    dq 0
.code: equ $ - gdt64
    dq (1<<43) | (1<<44) | (1<<47) | (1<<53)
.data: equ $ - gdt64
    dq (1<<44) | (1<<47) | (1<<41)
.pointer:
    dw $ - gdt64 - 1
    dq gdt64

; ==================== 64-bit Entry ====================
section .text
bits 64
long_mode_entry:
    mov ax, gdt64.data
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov rsp, stack_top

    mov rsi, boot_msg_long_mode
    call serial_write_64

    mov edi, [multiboot_magic_value]
    mov esi, [multiboot_info_ptr]
    call kernel_main

.hang:
    cli
    hlt
    jmp .hang

serial_putchar_64:
    mov dx, COM1_PORT + 5
.wait:
    in al, dx
    test al, 0x20
    jz .wait
    mov dx, COM1_PORT
    mov al, bl
    out dx, al
    ret

serial_write_64:
.next:
    lodsb
    test al, al
    jz .done
    cmp al, 10
    jne .emit
    mov bl, 13
    call serial_putchar_64
.emit:
    mov bl, al
    call serial_putchar_64
    jmp .next
.done:
    ret
