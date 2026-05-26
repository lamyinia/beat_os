#ifndef BEAT_OS_MEMORY_MEMORY_MAP_H
#define BEAT_OS_MEMORY_MEMORY_MAP_H

#include "drivers/log.h"

#define MEMORY_MAP_MAX_REGIONS 32
#define MEMORY_PAGE_SIZE 4096ULL

struct memory_region {
    uint64_t base;
    uint64_t length;
};

void memory_map_reset(void);
int memory_map_add_region(uint64_t base, uint64_t length);

uint64_t memory_map_region_count(void);
const struct memory_region *memory_map_regions(void);
uint64_t memory_map_usable_bytes(void);
uint64_t memory_map_usable_pages(void);

void memory_map_log_summary(void);

#endif
