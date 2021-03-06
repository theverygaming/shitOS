#pragma once
#include "memorymap.h"
#include "types.h"

namespace memalloc::page {
    void *phys_malloc(uint32_t blocks);
    void phys_free(void *adr);
    void phys_init(memorymap::SMAP_entry *e620_map, int e620_len);

    void *kernel_malloc(uint32_t blocks);
    void kernel_alloc(void *adr, uint32_t blocks);
    void kernel_free(void *adr);
    void kernel_init();
}