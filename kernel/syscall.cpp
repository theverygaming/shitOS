#include "syscall.h"
#include "multitasking.h"
#include "stdlib.h"
#include "stdio.h"
#include "drivers/keyboard.h"
#include "debug.h"
#include "../config.h"

void syscall::syscallHandler(isr::Registers* regs) {
    multitasking::process* currentProcess = multitasking::getCurrentProcess(); // never use this to modify registers, it will not work
    multitasking::context *current_context = (multitasking::context*)(KERNEL_VIRT_ADDRESS + REGISTER_STORE_OFFSET); // return values using this
    if(regs->eax == 1) { // sys_exit
        DEBUG_PRINTF("syscall: sys_exit code: %d\n", regs->ebx);
        multitasking::killCurrentProcess();
    }
    else if(regs->eax == 4) { // sys_write
        DEBUG_PRINTF("syscall: sys_write\n");
        char string[regs->edx + 1];
        string[regs->edx] = '\0';
        memcpy(string, (char*)regs->ecx, regs->edx);
        printf("%s", string);
    }
    else if(regs->eax == 3) { // sys_read
        DEBUG_PRINTF("syscall: sys_read\n");

        int bufStart = drivers::keyboard::bufferlocation;
        while(drivers::keyboard::bufferlocation - bufStart < (int)regs->edx) {
            drivers::keyboard::manualRead();
            if(drivers::keyboard::buffer[drivers::keyboard::bufferlocation] == '\n') {
                break;
            }
        }
        uint32_t readCharacters = drivers::keyboard::bufferlocation - bufStart;

        drivers::keyboard::bufferlocation = -1;
        s_memcpy((char*)regs->ecx, &drivers::keyboard::buffer[bufStart+1], readCharacters);
        current_context->eax = readCharacters;
    }
    else if(regs->eax == 2) { // sys_fork
        DEBUG_PRINTF("syscall: sys_fork\n");
        multitasking::process* newprocess = multitasking::fork_current_process();
        if(newprocess) {
            newprocess->registerContext.eax = 0;
            current_context->eax = newprocess->pid;
        }
        else {
            current_context->eax = -1; // this is probably the wrong return value
        }
    }
    else if(regs->eax == 11) { // sys_execve
        DEBUG_PRINTF("syscall: sys_execve\n");
        printf("%s\n", regs->ebx);
    }
    else {
        DEBUG_PRINTF("Unknown syscall %u\n", regs->eax);
        current_context->eax = -1; // return error - once again not the right return code
    }
}