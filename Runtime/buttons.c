/* Interrupt routine for tick count and handling the buttons */

#include "buttons.h"
#include "BIOS.h"
#include <stm32f10x.h>

// Milliseconds since boot, good for two months of operation.
// Also, most functions will handle overflows properly.
static volatile uint32_t TICKCOUNT = 0;

// Timestamp when was the last time some key was down
static volatile uint32_t KEYS_LAST_DOWN = 0;

// Flags for keys that have been pressed since the last time they were read
static volatile uint32_t KEYS_PRESSED = 0;

// Flags for keys that are currently down
static volatile uint32_t KEYS_DOWN = 0;

// Timestamp when was the last time KEYS_DOWN changed
static volatile uint32_t KEYS_LAST_CHANGED = 0;

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
    
    if (keys != KEYS_DOWN)
    {
        KEYS_DOWN = keys;
        KEYS_LAST_CHANGED = TICKCOUNT;
    }
    
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

// Check if some keys are being held down
// Returns the keys that are still down and the number of milliseconds they've
// been down.
uint32_t held_keys(uint32_t mask, uint32_t *ticks)
{
    uint32_t keys, time;
    do
    {
        time = KEYS_LAST_CHANGED;
        keys = KEYS_DOWN & mask;
    } while (time != KEYS_LAST_CHANGED);
    
    *ticks = TICKCOUNT - time;
    return keys;
}

// Get number of milliseconds passed since boot
uint32_t get_time()
{
    return TICKCOUNT;
}

void delay_ms(uint32_t milliseconds)
{
    uint32_t start = TICKCOUNT;
    while (TICKCOUNT - start < milliseconds);
}
