#pragma once
/* Prevent compiler from optimizing away a memory access.
 * Used to reset flags for USART/SPI by reading status register. */
#define always_read(x) asm(""::"r"(x))

/* Ordinary MAX() and MIN() macros */
#define MAX(x,y) ((x) > (y) ? (x) : (y))
#define MIN(x,y) ((x) < (y) ? (x) : (y))

/* Bit-banding access to single bits. */
#define BITBAND_ACCESS(variable, bitnumber) \
    *(uint32_t*)(((uint32_t)&variable & 0xF0000000) \
            + 0x2000000 \
            + (((uint32_t)&variable & 0x000FFFFF) << 5) \
            + (bitnumber << 2))

#define set_bit(variable, bitmask) \
    BITBAND_ACCESS(variable, __builtin_ctz(bitmask)) = 1

#define clear_bit(variable, bitmask) \
    BITBAND_ACCESS(variable, __builtin_ctz(bitmask)) = 0

#define read_bit(variable, bitmask) \
    BITBAND_ACCESS(variable, __builtin_ctz(bitmask))


