#ifndef BEAT_OS_MEMORY_PAGE_FRAME_ALLOCATOR_H
#define BEAT_OS_MEMORY_PAGE_FRAME_ALLOCATOR_H

#include "drivers/log.h"

void page_frame_allocator_init(uint64_t multiboot_info_addr);
uint64_t page_frame_allocator_alloc(void);

int page_frame_allocator_is_initialized(void);
uint64_t page_frame_allocator_first_page(void);
uint64_t page_frame_allocator_allocated_pages(void);
uint64_t page_frame_allocator_remaining_pages(void);

#endif
