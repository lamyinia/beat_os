#ifndef BEAT_OS_LOG_H
#define BEAT_OS_LOG_H

typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long      uint64_t;
typedef signed char        int8_t;
typedef signed short       int16_t;
typedef signed int         int32_t;
typedef signed long        int64_t;

enum log_level {
    LOG_INFO,
    LOG_WARN,
    LOG_PANIC,
};

void log_init(void);
void console_write(const char *str);
void console_write_hex64(uint64_t value);
void log_line(enum log_level level, const char *message);
void panic(const char *message);

#endif
