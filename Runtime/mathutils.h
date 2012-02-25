#pragma once

#define abs(x) (((x)>0) ? (x) : -(x))

// Integer division, return ceiling of the result.
static inline uint32_t div_round_up(uint32_t x, uint32_t y)
{
    return (x - 1) / y + 1;
}

// Integer division, round to nearest (for positive numbers)
static inline uint32_t div_round(uint32_t x, uint32_t y)
{
    return (x + y / 2) / y;
}
