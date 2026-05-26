#ifndef BEAT_OS_BOOT_MULTIBOOT2_H
#define BEAT_OS_BOOT_MULTIBOOT2_H

#include "drivers/log.h"

#define MULTIBOOT2_BOOTLOADER_MAGIC 0x36D76289

void multiboot2_log_info(uint32_t magic, uint64_t info_addr);
void multiboot2_init_memory_map(uint32_t magic, uint64_t info_addr);

#endif
