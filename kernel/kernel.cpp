#include "../config.h"
#include "cpubasics.h"
#include "drivers/keyboard.h"
#include "drivers/serial.h"
#include "elf.h"
#include "gdt.h"
#include "idt.h"
#include "isr.h"
#include "memalloc.h"
#include "memorymap.h"
#include "multitasking.h"
#include "paging.h"
#include "stdio.h"
#include "stdlib.h"
#include "syscall.h"
#include "drivers/pci.h"

void kernelstart();

void _start(void) {
    kernelstart();
}

// very important arrays definitely
uint8_t zerotwo[18][13] = {
    {15, 15, 15, 15, 13, 13, 13, 13, 13, 15, 15, 15, 15},
    {15, 15, 15, 13, 13, 13, 13, 13, 13, 13, 15, 15, 15},
    {15, 15, 13, 13, 4, 13, 13, 13, 4, 13, 13, 15, 15},
    {15, 15, 13, 7, 4, 7, 7, 7, 4, 7, 13, 15, 15},
    {15, 15, 13, 13, 13, 13, 13, 13, 13, 13, 13, 15, 15},
    {15, 15, 13, 13, 14, 14, 14, 14, 14, 13, 13, 15, 15},
    {15, 15, 13, 13, 14, 2, 14, 2, 14, 13, 13, 15, 15},
    {15, 15, 13, 13, 14, 14, 14, 14, 14, 13, 13, 15, 15},
    {15, 15, 13, 13, 0, 15, 14, 15, 0, 13, 13, 15, 15},
    {15, 15, 7, 7, 15, 15, 6, 15, 15, 7, 7, 15, 15},
    {15, 15, 4, 8, 4, 15, 6, 15, 4, 8, 4, 15, 15},
    {15, 0, 4, 8, 4, 4, 8, 4, 4, 8, 4, 0, 15},
    {0, 14, 14, 0, 4, 4, 4, 4, 4, 0, 14, 14, 0},
    {0, 14, 14, 0, 4, 4, 8, 4, 4, 0, 14, 14, 0},
    {15, 0, 0, 8, 8, 7, 4, 7, 8, 8, 0, 0, 15},
    {15, 15, 15, 0, 4, 4, 0, 4, 4, 0, 15, 15, 15},
    {15, 15, 15, 0, 4, 4, 0, 4, 4, 0, 15, 15, 15},
    {15, 15, 15, 15, 0, 0, 15, 0, 0, 15, 15, 15, 15},
};

uint8_t franxxlogo[9][18] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 4, 0},
    {0, 0, 0, 0, 0, 0, 0, 15, 0, 15, 0, 0, 0, 4, 0, 4, 0, 0},
    {0, 0, 1, 0, 1, 0, 1, 0, 15, 0, 4, 0, 4, 0, 4, 0, 0, 0},
    {0, 0, 0, 1, 0, 1, 0, 1, 0, 4, 0, 4, 0, 4, 0, 0, 0, 0},
    {0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 4, 0, 4, 0, 0, 0, 0, 0},
    {0, 0, 0, 1, 0, 1, 0, 1, 0, 4, 0, 4, 0, 4, 0, 0, 0, 0},
    {0, 0, 1, 0, 1, 0, 1, 0, 15, 0, 4, 0, 4, 0, 4, 0, 0, 0},
    {0, 1, 0, 1, 0, 0, 0, 15, 0, 15, 0, 0, 0, 4, 0, 4, 0, 0},
    {1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
};

void kernelstart() {
    for (uint32_t i = 0; i < 0xFFFFFF; i++) {}
    clrscr();
    drivers::serial::init();
    printf("hewwo\n");
    memorymap::initMemoryMap((void *)0x7C00 + 0x7000, (void *)0x7C00 + 0x7004);
    paging::clearPageTables((void *)0x0, KERNEL_VIRT_ADDRESS / 4096);
    memalloc::page::phys_init(memorymap::map_entries, memorymap::map_entrycount);
    memalloc::page::kernel_init();
    memalloc::page::kernel_alloc((void *)(KERNEL_VIRT_ADDRESS + KERNEL_FREE_AREA_BEGIN_OFFSET), 245);
    cpubasics::cpuinit();
    drivers::keyboard::init();
    isr::RegisterHandler(0x80, syscall::syscallHandler);
    //elf::load_program((void *)(KERNEL_VIRT_ADDRESS + KERNEL_FREE_AREA_BEGIN_OFFSET));
    memalloc::page::kernel_free((void *)(KERNEL_VIRT_ADDRESS + KERNEL_FREE_AREA_BEGIN_OFFSET));
    drivers::pci::init();

    for (int i = 0; i < 18; i++) {
        for (int j = 0; j < 13; j++) {
            putcolor(j + 67, i, zerotwo[i][j] << 4 | 0);
        }
    }

    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 18; j++) {
            putcolor(j + 49, i, franxxlogo[i][j] << 4 | 7);
        }
    }

    multitasking::process_pagerange prange[PROCESS_MAX_PAGE_RANGES];
    multitasking::createPageRange(prange);
    for (int i = 0; i < PROCESS_MAX_PAGE_RANGES; i++) {
        if (prange[i].pages != 0) {
            printf("created prange, pb: 0x%p vb: 0x%p pg: %u, i: %d\n", prange[i].phys_base, prange[i].virt_base, prange[i].pages, i);
        }
    }

    int counter = 0;
    while (true) {
        *((unsigned char *)((KERNEL_VIRT_ADDRESS + VIDMEM_OFFSET) + 2 * 70 + 160 * 0)) = counter / 20;
        counter++;
        if (counter == 1000) {
            counter = 0;
        }
    }
}