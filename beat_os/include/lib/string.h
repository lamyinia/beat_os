#ifndef BEAT_OS_LIB_STRING_H
#define BEAT_OS_LIB_STRING_H

typedef unsigned long size_t;

void *memset(void *dest, int value, size_t count);
void *memcpy(void *dest, const void *src, size_t count);
int memcmp(const void *lhs, const void *rhs, size_t count);
size_t strlen(const char *str);

#endif
