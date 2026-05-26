#include "boot/multiboot2.h"
#include "drivers/log.h"
#include "interrupt/idt.h"
#include "test/test.h"

static inline void halt_forever(void) {
    for (;;) {
        __asm__ volatile ("cli; hlt");
    }
}

// Entry point called from boot.asm
void kernel_main(uint32_t multiboot_magic, uint64_t multiboot_info_addr) {
    static const uint64_t VGA_BUFFER_ADDR = 0xB8000;
    static const uint64_t COM1_PORT_ADDR = 0x3F8;
    int failures;

    log_init();

    log_line(LOG_INFO, "kernel: console ready");
    log_line(LOG_INFO, "kernel: entered kernel_main");
    log_line(LOG_INFO, "kernel: x86_64 kernel loaded via Multiboot2 + GRUB");

    console_write("[INFO] kernel: VGA buffer @ ");
    console_write_hex64(VGA_BUFFER_ADDR);
    console_write("\n");

    console_write("[INFO] kernel: COM1 port @ ");
    console_write_hex64(COM1_PORT_ADDR);
    console_write("\n");

    multiboot2_log_info(multiboot_magic, multiboot_info_addr);
    multiboot2_init_memory_map(multiboot_magic, multiboot_info_addr);

    idt_init();
    log_line(LOG_INFO, "kernel: IDT initialized");

    log_line(LOG_INFO, "kernel: entering test mode");
    failures = run_tests();

    if (failures == 0) {
        log_line(LOG_INFO, "kernel: test run complete");
    } else {
        log_line(LOG_PANIC, "kernel: test run failed");
    }

    halt_forever();
}
