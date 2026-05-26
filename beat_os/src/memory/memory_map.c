#include "memory/memory_map.h"

static struct memory_region memory_regions[MEMORY_MAP_MAX_REGIONS];
static uint64_t memory_region_count_value;

static uint64_t align_up_page(uint64_t value) {
    return (value + (MEMORY_PAGE_SIZE - 1)) & ~(MEMORY_PAGE_SIZE - 1);
}

static uint64_t align_down_page(uint64_t value) {
    return value & ~(MEMORY_PAGE_SIZE - 1);
}

static uint64_t aligned_region_base(const struct memory_region *region) {
    return align_up_page(region->base);
}

static uint64_t aligned_region_end(const struct memory_region *region) {
    return align_down_page(region->base + region->length);
}

static uint64_t aligned_region_length(const struct memory_region *region) {
    uint64_t aligned_base = aligned_region_base(region);
    uint64_t aligned_end = aligned_region_end(region);

    if (aligned_end <= aligned_base) {
        return 0;
    }

    return aligned_end - aligned_base;
}

static void write_field_hex64(const char *label, uint64_t value) {
    console_write("[INFO] memory: ");
    console_write(label);
    console_write("=");
    console_write_hex64(value);
    console_write("\n");
}

void memory_map_reset(void) {
    memory_region_count_value = 0;
}

int memory_map_add_region(uint64_t base, uint64_t length) {
    if (length == 0 || memory_region_count_value >= MEMORY_MAP_MAX_REGIONS) {
        return 0;
    }

    memory_regions[memory_region_count_value].base = base;
    memory_regions[memory_region_count_value].length = length;
    memory_region_count_value++;
    return 1;
}

uint64_t memory_map_region_count(void) {
    return memory_region_count_value;
}

const struct memory_region *memory_map_regions(void) {
    return memory_regions;
}

uint64_t memory_map_usable_bytes(void) {
    uint64_t total = 0;
    uint64_t i;

    for (i = 0; i < memory_region_count_value; i++) {
        total += aligned_region_length(&memory_regions[i]);
    }

    return total;
}

uint64_t memory_map_usable_pages(void) {
    return memory_map_usable_bytes() / MEMORY_PAGE_SIZE;
}

void memory_map_log_summary(void) {
    uint64_t i;

    log_line(LOG_INFO, "memory: usable region summary");
    write_field_hex64("usable_region_count", memory_region_count_value);
    write_field_hex64("usable_bytes", memory_map_usable_bytes());
    write_field_hex64("usable_pages", memory_map_usable_pages());

    for (i = 0; i < memory_region_count_value; i++) {
        const struct memory_region *region = &memory_regions[i];
        uint64_t aligned_base = aligned_region_base(region);
        uint64_t aligned_length = aligned_region_length(region);

        console_write("[INFO] memory: usable region base=");
        console_write_hex64(region->base);
        console_write(" length=");
        console_write_hex64(region->length);
        console_write(" aligned_base=");
        console_write_hex64(aligned_base);
        console_write(" aligned_length=");
        console_write_hex64(aligned_length);
        console_write("\n");
    }
}
