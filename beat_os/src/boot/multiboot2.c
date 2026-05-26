#include "boot/multiboot2.h"
#include "lib/string.h"
#include "memory/memory_map.h"

#define MULTIBOOT_TAG_TYPE_END 0
#define MULTIBOOT_TAG_TYPE_MMAP 6
#define MULTIBOOT_MEMORY_AVAILABLE 1
#define MULTIBOOT_MEMORY_ACPI_RECLAIMABLE 3
#define MULTIBOOT_MEMORY_NVS 4
#define MULTIBOOT_MEMORY_BADRAM 5

struct multiboot_info_header {
    uint32_t total_size;
    uint32_t reserved;
} __attribute__((packed));

struct multiboot_tag {
    uint32_t type;
    uint32_t size;
} __attribute__((packed));

struct multiboot_tag_mmap {
    uint32_t type;
    uint32_t size;
    uint32_t entry_size;
    uint32_t entry_version;
} __attribute__((packed));

struct multiboot_mmap_entry {
    uint64_t base_addr;
    uint64_t length;
    uint32_t type;
    uint32_t reserved;
} __attribute__((packed));

static uint64_t align_up_8(uint64_t value) {
    return (value + 7) & ~((uint64_t)7);
}

static const char *memory_type_name(uint32_t type) {
    switch (type) {
    case MULTIBOOT_MEMORY_AVAILABLE:
        return "available";
    case 2:
        return "reserved";
    case MULTIBOOT_MEMORY_ACPI_RECLAIMABLE:
        return "acpi reclaimable";
    case MULTIBOOT_MEMORY_NVS:
        return "acpi nvs";
    case MULTIBOOT_MEMORY_BADRAM:
        return "bad ram";
    default:
        return "firmware-defined";
    }
}

static void write_field_hex64(const char *label, uint64_t value) {
    console_write("[INFO] multiboot2: ");
    console_write(label);
    console_write("=");
    console_write_hex64(value);
    console_write("\n");
}

static void write_mmap_entry(const struct multiboot_mmap_entry *entry) {
    console_write("[INFO] multiboot2: memory region base=");
    console_write_hex64(entry->base_addr);
    console_write(" length=");
    console_write_hex64(entry->length);
    console_write(" end=");
    console_write_hex64(entry->base_addr + entry->length);
    console_write(" type=");
    console_write(memory_type_name(entry->type));
    console_write("\n");
}

static const struct multiboot_tag_mmap *find_memory_map_tag(
    const struct multiboot_info_header *header) {
    const struct multiboot_tag *tag;
    uint64_t offset = sizeof(struct multiboot_info_header);

    while (offset + sizeof(struct multiboot_tag) <= header->total_size) {
        tag = (const struct multiboot_tag *)((const uint8_t *)header + offset);
        if (tag->type == MULTIBOOT_TAG_TYPE_END) {
            break;
        }
        if (tag->type == MULTIBOOT_TAG_TYPE_MMAP) {
            return (const struct multiboot_tag_mmap *)tag;
        }
        offset = align_up_8(offset + tag->size);
    }

    return 0;
}

static const struct multiboot_info_header *validated_multiboot_header(
    uint32_t magic, uint64_t info_addr) {
    if (magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
        log_line(LOG_PANIC, "multiboot2: invalid bootloader magic");
        return 0;
    }

    if (info_addr == 0) {
        log_line(LOG_PANIC, "multiboot2: missing boot information pointer");
        return 0;
    }

    return (const struct multiboot_info_header *)info_addr;
}

static void log_memory_map(const struct multiboot_tag_mmap *mmap_tag) {
    uint64_t entry_offset = sizeof(struct multiboot_tag_mmap);

    console_write("[INFO] multiboot2: memory map\n");
    write_field_hex64("mmap.entry_size", mmap_tag->entry_size);
    write_field_hex64("mmap.entry_version", mmap_tag->entry_version);

    while (entry_offset + mmap_tag->entry_size <= mmap_tag->size) {
        const struct multiboot_mmap_entry *entry =
            (const struct multiboot_mmap_entry *)((const uint8_t *)mmap_tag +
                                                  entry_offset);
        write_mmap_entry(entry);
        entry_offset += mmap_tag->entry_size;
    }
}

void multiboot2_log_info(uint32_t magic, uint64_t info_addr) {
    const struct multiboot_info_header *header;
    const struct multiboot_tag_mmap *mmap_tag;

    log_line(LOG_INFO, "multiboot2: checking boot information");
    write_field_hex64("magic", magic);
    write_field_hex64("info_addr", info_addr);

    header = validated_multiboot_header(magic, info_addr);
    if (header == 0) {
        return;
    }
    write_field_hex64("total_size", header->total_size);

    mmap_tag = find_memory_map_tag(header);
    if (mmap_tag == 0) {
        log_line(LOG_WARN, "multiboot2: memory map tag not found");
        return;
    }

    log_memory_map(mmap_tag);
}

void multiboot2_init_memory_map(uint32_t magic, uint64_t info_addr) {
    const struct multiboot_info_header *header;
    const struct multiboot_tag_mmap *mmap_tag;
    uint64_t entry_offset = sizeof(struct multiboot_tag_mmap);

    memory_map_reset();

    header = validated_multiboot_header(magic, info_addr);
    if (header == 0) {
        return;
    }

    mmap_tag = find_memory_map_tag(header);
    if (mmap_tag == 0) {
        log_line(LOG_WARN, "memory: no multiboot memory map available");
        return;
    }

    while (entry_offset + mmap_tag->entry_size <= mmap_tag->size) {
        const struct multiboot_mmap_entry *entry =
            (const struct multiboot_mmap_entry *)((const uint8_t *)mmap_tag +
                                                  entry_offset);
        if (entry->type == MULTIBOOT_MEMORY_AVAILABLE &&
            !memory_map_add_region(entry->base_addr, entry->length)) {
            log_line(LOG_WARN, "memory: usable region table is full");
            break;
        }
        entry_offset += mmap_tag->entry_size;
    }

    memory_map_log_summary();
}
