/* Interrupt routine for tick count and handling the buttons */

#include "buttons.h"
#include "BIOS.h"
#include <stm32f10x.h>
#include "irq.h"

// Milliseconds since boot, good for two months of operation.
// Also, most functions will handle overflows properly.
static volatile uint32_t TICKCOUNT = 0;

// Timestamp when was the last time some key was down
static volatile uint32_t KEYS_LAST_DOWN = 0;

// Flags for keys that have been pressed since the last time they were read
static volatile uint32_t KEYS_PRESSED = 0;

// Debounce time for keys, in milliseconds
#define DEBOUNCE 10

void __irq__ TIM3_IRQHandler(void)
{ 
    TIM3->SR = 0; // Clear interrupt flag
    
    TICKCOUNT++;
    
    uint32_t keys = (~__Get(KEY_STATUS)) & ALL_KEYS;
    
    // Only record keypresses the first time the key goes down
    if (keys && TICKCOUNT - KEYS_LAST_DOWN > DEBOUNCE)
        KEYS_PRESSED |= keys;
    
    if (keys) KEYS_LAST_DOWN = TICKCOUNT;
    
    TimerTick();
}

// Check if some keys have been pressed, but don't clear the key status.
uint32_t peek_keys(uint32_t mask)
{
    return KEYS_PRESSED & mask;
}

// Check if some keys have been pressed, and clear the status for them.
uint32_t get_keys(uint32_t mask)
{
    return __sync_fetch_and_and(&KEYS_PRESSED, ~mask) & mask;
}

// Get number of milliseconds passed since boot
uint32_t get_time()
{
    return TICKCOUNT;
}

void delay_ms(uint32_t milliseconds)
{
    uint32_t end = TICKCOUNT + milliseconds;
    while (TICKCOUNT < end);
}
