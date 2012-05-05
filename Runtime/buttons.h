#pragma once

#include <stdint.h>
#include "irq.h"

/* Functions for accessing the button status and timer ticks, which are
 * updated in a 1ms interval interrupt routine.
 */

// This can be replaced elsewhere to execute custom code every ms
// The call will come from an interrupt context.
void TimerTick(); // {}

// Check if some keys have been pressed, but don't clear the key status.
uint32_t peek_keys(uint32_t mask);

// Check if some keys have been pressed, and clear the status for them.
uint32_t get_keys(uint32_t mask);

// Check if some keys are being held down
// Returns the keys that are still down and the number of milliseconds they've
// been down.
uint32_t held_keys(uint32_t mask, uint32_t *ticks);

// Get number of milliseconds passed since boot
uint32_t get_time();

// Delay a specified number of milliseconds in a while loop
void delay_ms(uint32_t milliseconds);

// Get scroller increment based on held_keys time. This achieves incremental
// acceleration.
int scroller_speed();

#define BUTTON1        0x4000    
#define BUTTON2        0x2000    
#define BUTTON3        0x0100
#define BUTTON4        0x0200

#define SCROLL1_LEFT   0x0400
#define SCROLL1_RIGHT  0x0800
#define SCROLL1_PRESS  0x1000

#define SCROLL2_LEFT   0x0008
#define SCROLL2_RIGHT  0x8000
#define SCROLL2_PRESS  0x0040

#define ANY_KEY        0xFFFF