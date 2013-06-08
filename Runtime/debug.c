/* Tools for handling crashes & debugging */

#include <stdbool.h>
#include <stdint.h>
#include "BIOS.h"
#include <stdio.h>
#include "stm32f10x.h"
#include "ff.h"
#include "irq.h"

/* Memory dumps */

struct {
    uint32_t REGS[16];
    char BUILDID[32];
} CRASHDATA;

const char BUILDID[32] = __DATE__ " " __TIME__;

static FIL memdumpfile;

static bool make_memdump()
{
    memcpy(CRASHDATA.BUILDID, BUILDID, sizeof(BUILDID));
    
    unsigned bytes;
    f_open(&memdumpfile, "memory.dmp", FA_WRITE | FA_CREATE_ALWAYS);
    f_write(&memdumpfile, (void*)0x20000000, 0xC000, &bytes);
    return (f_close(&memdumpfile) == FR_OK && bytes == 0xC000);
}

/* Stack parsing */

extern uint32_t _scode, _ecode, _estack;

// Find newest 4 code addresses on the stack, they are probably the
// previous functions on call stack.
void print_callers(uint32_t *sp)
{
    uint32_t callers[4] = {0};
    int i = 0;
    uint32_t *p = sp;
    while (i < 4 && p < &_estack)
    {
        if (*p >= (uint32_t)&_scode && *p <= (uint32_t)&_ecode)
            callers[i++] = *p;
        
        p++;
    }
    fprintf(stderr, "Callers: %08lx %08lx %08lx %08lx\n",
            callers[0], callers[1], callers[2], callers[3]);
}

/* Soft crashes (asserts) */

// Message is free-form error message,
// caller is the value from __builtin_return_address(0).
void crash_with_message(const char *message, void *caller)
{
    __Clear_Screen(0b0000000000011111);
    __Set(BEEP_VOLUME, 0);
    
    fprintf(stderr, "   %s   \n", message);
    fprintf(stderr, "Caller: %08lx\n", (uint32_t)caller);
    
    print_callers(__builtin_frame_address(0));
    
    if (make_memdump())
        fprintf(stderr, "Memory dump saved to memory.dmp.\n");
    else
        fprintf(stderr, "Memory dump failed.\n");
    
    while(1);
}

void __assert_fail(const char *assert, const char *file, unsigned int line)
{
    char tmp[50];
    snprintf(tmp, 50, "assert failed: %s:%d: %s", file, line, assert);
    crash_with_message(tmp, __builtin_return_address(0));
}

static uint32_t *SP;

/* Hard crashes (invalid memory access etc.) */

void __irq__ print_hardfault()
{
    __Clear_Screen(0b0000000000011111);
    __Set(BEEP_VOLUME, 0);
    
    fprintf(stderr, "\n\n   HARDFAULT   \n");
    fprintf(stderr, "SP:%08lx PC:%08lx LR:%08lx\n",
            CRASHDATA.REGS[13], CRASHDATA.REGS[15], CRASHDATA.REGS[14]);
    fprintf(stderr, "SCB HFSR:%08lx CFSR:%08lx BFAR:%08lx\n\n",
            SCB->HFSR, SCB->CFSR, SCB->BFAR);
    
    fprintf(stderr, "R0:%08lx R1:%08lx R2:%08lx R3:%08lx\n",
            CRASHDATA.REGS[0], CRASHDATA.REGS[1], CRASHDATA.REGS[2], CRASHDATA.REGS[3]);
    fprintf(stderr, "R4:%08lx R5:%08lx R6:%08lx R7:%08lx\n",
            CRASHDATA.REGS[4], CRASHDATA.REGS[5], CRASHDATA.REGS[6], CRASHDATA.REGS[7]);
    fprintf(stderr, "R8:%08lx R9:%08lx\n",
            CRASHDATA.REGS[8], CRASHDATA.REGS[9]);
    fprintf(stderr, "R10:%08lx R11:%08lx R12:%08lx\n",
            CRASHDATA.REGS[10], CRASHDATA.REGS[11], CRASHDATA.REGS[12]);
    fprintf(stderr, "2000b800: %08lx %08lx %08lx\n\n",
            *(uint32_t*)0x2000b800, *(uint32_t*)0x2000b804, *(uint32_t*)0x2000b808); 
    
    print_callers((uint32_t*)CRASHDATA.REGS[13]);
    
    if (make_memdump())
        fprintf(stderr, "Memory dump saved to memory.dmp.\n");
    else
        fprintf(stderr, "Memory dump failed.\n");
    while(1);
}

void __attribute__((naked, externally_visible)) HardFaultException()
{
    // Rescue stack pointer and register values
    asm("mrs %0, msp" : "=r"(SP) : :);
    asm("str r4, %0" : "=m"(CRASHDATA.REGS[4]) : :);
    asm("str r5, %0" : "=m"(CRASHDATA.REGS[5]) : :);
    asm("str r6, %0" : "=m"(CRASHDATA.REGS[6]) : :);
    asm("str r7, %0" : "=m"(CRASHDATA.REGS[7]) : :);
    asm("str r8, %0" : "=m"(CRASHDATA.REGS[8]) : :);
    asm("str r9, %0" : "=m"(CRASHDATA.REGS[9]) : :);
    asm("str r10, %0" : "=m"(CRASHDATA.REGS[10]) : :);
    asm("str r11, %0" : "=m"(CRASHDATA.REGS[11]) : :);
    
    CRASHDATA.REGS[0] = SP[0];
    CRASHDATA.REGS[1] = SP[1];
    CRASHDATA.REGS[2] = SP[2];
    CRASHDATA.REGS[3] = SP[3];
    CRASHDATA.REGS[12] = SP[4];
    CRASHDATA.REGS[13] = (uint32_t)(SP + 8);
    CRASHDATA.REGS[14] = SP[5];
    CRASHDATA.REGS[15] = SP[6];
    
    // Print message to screen and halt.
    print_hardfault();
}



