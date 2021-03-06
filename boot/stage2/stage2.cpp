void stage2start(void);

void _start(void) {
    stage2start();
}

#include "../../config.h"
#include "gdt.h"
#include "hdd.h"
#include "paging.h"
#include "stdio.h"

int check_a20() {
    uint32_t *ptr1 = (uint32_t *)0x2ea5000;
    uint32_t *ptr2 = (uint32_t *)0x2fa5000;
    *ptr1 = 420;
    if (*ptr2 == 420) {
        return -1;
    }
    return 0;
}

void deth_loop() {
    while (true) {
        asm volatile("cli");
        asm volatile("hlt");
    }
}

void stage2start(void) {
    clrscr();
    if (check_a20() != 0) {
        printf("Couldn't enable A20 -- unable to start kernel like this\nRIP\n");
        deth_loop();
    }
    paging::initpaging();
    gdt::i686_GDT_Initialize();
    printf("Searching for hard drives\n");
    hdd::generic::scanDrives();
    if (hdd::generic::alldrives[0].alive) {
        printf("Trying to load data from HDD...\n");
        hdd::generic::readDrive((void *)KERNEL_VIRT_ADDRESS, 21, 2027, hdd::generic::alldrives[0]);
        hdd::generic::readDrive((void *)(KERNEL_VIRT_ADDRESS + KERNEL_FREE_AREA_BEGIN_OFFSET), 2048, 4690, hdd::generic::alldrives[0]);
        printf("Jumping to kernel\n");
        uint32_t sp_adr = KERNEL_VIRT_ADDRESS + KERNEL_START_STACK_POINTER_OFFSET;
        uint32_t kerneladr = KERNEL_VIRT_ADDRESS;
        asm volatile("" : : "a"(sp_adr));
        asm("mov %eax, %esp");
        asm volatile("" : : "a"(kerneladr));
        asm("jmpl %eax");
    } else {
        printf("no IDE hdd found. go get one\nSystem halted\n");
        deth_loop();
    }
}