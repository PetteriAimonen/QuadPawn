/* Some Fourier transforms */

#include "fix16.h"
#include "amx.h"

// Packed Pawn arrays are indexed a bit funnily
#define IDX(x) (((x) & 0xFFFFFFC) | (3 - ((x) & 3)))

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
        fix16_t value = input[IDX(i)] * 256;
        
        // Calculate value * (cos(angle) - i * sin(angle)) and add to sum.
        real += fix16_mul(value, fix16_cos(angle));
        imag += fix16_mul(value, fix16_sin(angle));
        
        angle += step;
    }
    
    fix16_t scale = count * 256;
    *realp = fix16_div(real, scale);
    *imagp = fix16_div(imag, scale);
    
    return 0;
}

int amxinit_fourier(AMX *amx)
{
    static const AMX_NATIVE_INFO funcs[] = {
        {"dft", amx_dft},
        {0, 0}
    };
    
    return amx_Register(amx, funcs, -1);
}