#include "lib/string.h"

void *memset(void *dest, int value, size_t count) {
    unsigned char *bytes = (unsigned char *)dest;
    size_t i;

    for (i = 0; i < count; i++) {
        bytes[i] = (unsigned char)value;
    }

    return dest;
}

void *memcpy(void *dest, const void *src, size_t count) {
    unsigned char *dst_bytes = (unsigned char *)dest;
    const unsigned char *src_bytes = (const unsigned char *)src;
    size_t i;

    for (i = 0; i < count; i++) {
        dst_bytes[i] = src_bytes[i];
    }

    return dest;
}

int memcmp(const void *lhs, const void *rhs, size_t count) {
    const unsigned char *lhs_bytes = (const unsigned char *)lhs;
    const unsigned char *rhs_bytes = (const unsigned char *)rhs;
    size_t i;

    for (i = 0; i < count; i++) {
        if (lhs_bytes[i] != rhs_bytes[i]) {
            return (int)lhs_bytes[i] - (int)rhs_bytes[i];
        }
    }

    return 0;
}

size_t strlen(const char *str) {
    size_t length = 0;

    while (str[length] != '\0') {
        length++;
    }

    return length;
}
