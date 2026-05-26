#include "drivers/log.h"

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void halt_forever(void) {
    for (;;) {
        __asm__ volatile ("cli; hlt");
    }
}

static const uint16_t COM1_PORT = 0x3F8;
static volatile uint16_t *const VGA_BUFFER = (volatile uint16_t *)0xB8000;
static const int VGA_COLS = 80;
static const int VGA_ROWS = 25;
static const uint8_t VGA_COLOR = 0x0F;

static int cursor_col = 0;
static int cursor_row = 0;

static void serial_init(void) {
    outb(COM1_PORT + 1, 0x00);
    outb(COM1_PORT + 3, 0x80);
    outb(COM1_PORT + 0, 0x03);
    outb(COM1_PORT + 1, 0x00);
    outb(COM1_PORT + 3, 0x03);
    outb(COM1_PORT + 2, 0xC7);
    outb(COM1_PORT + 4, 0x0B);
}

static void serial_putchar(char c) {
    while ((inb(COM1_PORT + 5) & 0x20) == 0) {
    }
    outb(COM1_PORT, (uint8_t)c);
}

static void vga_putchar(char c) {
    if (c == '\n') {
        cursor_col = 0;
        cursor_row++;
        return;
    }

    {
        int offset = cursor_row * VGA_COLS + cursor_col;
        VGA_BUFFER[offset] = (uint16_t)(c) | ((uint16_t)VGA_COLOR << 8);
    }

    cursor_col++;
    if (cursor_col >= VGA_COLS) {
        cursor_col = 0;
        cursor_row++;
    }

    if (cursor_row >= VGA_ROWS) {
        int row;
        int col;

        for (row = 0; row < VGA_ROWS - 1; row++) {
            for (col = 0; col < VGA_COLS; col++) {
                int dst = row * VGA_COLS + col;
                int src = (row + 1) * VGA_COLS + col;
                VGA_BUFFER[dst] = VGA_BUFFER[src];
            }
        }

        for (col = 0; col < VGA_COLS; col++) {
            VGA_BUFFER[(VGA_ROWS - 1) * VGA_COLS + col] =
                (uint16_t)' ' | ((uint16_t)VGA_COLOR << 8);
        }

        cursor_row = VGA_ROWS - 1;
        cursor_col = 0;
    }
}

static void vga_clear(void) {
    int i;
    for (i = 0; i < VGA_COLS * VGA_ROWS; i++) {
        VGA_BUFFER[i] = (uint16_t)' ' | ((uint16_t)VGA_COLOR << 8);
    }
    cursor_col = 0;
    cursor_row = 0;
}

void log_init(void) {
    serial_init();
    vga_clear();
}

void console_write(const char *str) {
    while (*str) {
        if (*str == '\n') {
            serial_putchar('\r');
        }
        serial_putchar(*str);
        vga_putchar(*str);
        str++;
    }
}

void console_write_hex64(uint64_t value) {
    static const char HEX[] = "0123456789ABCDEF";
    int shift;
    char digit[2];

    digit[1] = '\0';
    console_write("0x");
    for (shift = 60; shift >= 0; shift -= 4) {
        digit[0] = HEX[(value >> shift) & 0xF];
        console_write(digit);
    }
}

static void log_prefix(enum log_level level) {
    if (level == LOG_INFO) {
        console_write("[INFO] ");
    } else if (level == LOG_WARN) {
        console_write("[WARN] ");
    } else {
        console_write("[PANIC] ");
    }
}

void log_line(enum log_level level, const char *message) {
    log_prefix(level);
    console_write(message);
    console_write("\n");
}

void panic(const char *message) {
    log_line(LOG_PANIC, message);
    halt_forever();
}
