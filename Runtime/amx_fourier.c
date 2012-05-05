/* Some Fourier transforms */

#include <stdbool.h>
#include "fix16.h"
#include "amx.h"

// Packed Pawn arrays are indexed a bit funnily
#define INPUT_INDEX(x) (((x) & 0xFFFFFFC) | (3 - ((x) & 3)))

/* A basic single-frequency DFT, useful when you are interested in just a single signal. */
static cell AMX_NATIVE_CALL amx_dft(AMX *amx, const cell *params)
{
    // dft(input{}, Fixed: &real, Fixed: &imag, Fixed: period, count);
    uint8_t *input = (uint8_t*)params[1];
    int count = params[5];
    fix16_t period = params[4];
    fix16_t *realp = (fix16_t*)params[2];
    fix16_t *imagp = (fix16_t*)params[3];
    
    // Round the count to a multiple of period
    int multiple = fix16_from_int(count) / period;
    count = fix16_to_int(fix16_mul(fix16_from_int(multiple), period));
    
    fix16_t real = 0;
    fix16_t imag = 0;
    fix16_t step = fix16_div(2 * fix16_pi, period);
    fix16_t angle = 0;
    
    for (int i = 0; i < count; i++)
    {
        // We scale by 256 to achieve a good compromise between precision and
        // range.
        fix16_t value = input[INPUT_INDEX(i)] * 256;
        
        // Calculate value * (cos(angle) - i * sin(angle)) and add to sum.
        real += fix16_mul(value, fix16_cos(angle));
        imag += fix16_mul(value, -fix16_sin(angle));
        
        angle += step;
    }
    
    fix16_t scale = count * 256;
    *realp = fix16_div(real, scale);
    *imagp = fix16_div(imag, scale);
    
    return 0;
}

/* Fast Fourier Transform, with optional Hamming windowing. */

// Hackish way to calculate the window function on-the-fly.
static bool do_window;
static uint8_t *input_base;
static unsigned input_size;

static fix16_t input_convert(uint8_t *x)
{
    fix16_t value = ((*x) << 8);
    if (do_window)
    {
        int index = INPUT_INDEX(x - input_base);
        fix16_t w = 35389 - fix16_mul(30147, fix16_cos(2 * fix16_pi * index / input_size));
        value = fix16_mul(value, w);
    }
    return value;
}

#define INPUT_CONVERT(x) input_convert(&(x))

#include "fix16_fft.c"

static cell AMX_NATIVE_CALL amx_fft(AMX *amx, const cell *params)
{
    uint8_t *input = (uint8_t*)params[1];
    fix16_t *real = (fix16_t*)params[2];
    fix16_t *imag = (fix16_t*)params[3];
    unsigned count = params[4];
    
    do_window = params[5];
    input_base = input;
    input_size = count;
    
    fix16_fft(input, real, imag, count);
    
    return 0;
}

int amxinit_fourier(AMX *amx)
{
    static const AMX_NATIVE_INFO funcs[] = {
        {"dft", amx_dft},
        {"fft", amx_fft},
        {0, 0}
    };
    
    return amx_Register(amx, funcs, -1);
}