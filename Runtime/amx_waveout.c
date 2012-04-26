/* Functions for accessing the DAC from Pawn. */

#include "amx.h"
#include "BIOS.h"
#include "stm32f10x.h"
#include "mathutils.h"
#include "fix16.h"

static cell AMX_NATIVE_CALL amx_waveout_analog(AMX *amx, const cell *params)
{
    cell *buffer = (cell*)params[1];
    int samplerate = params[2];
    int count = params[3];
    
    int prescale = (CPUFREQ / 65536) / samplerate;
    int arr = div_round_up(CPUFREQ / (prescale + 1), samplerate) - 1;
    
    DAC->CR = 0;
    TIM7->PSC = prescale;
    DMA2_Channel4->CCR &= ~DMA_CCR1_EN;
    DMA2_Channel4->CMAR = (uint32_t)buffer;
    DMA2_Channel4->CNDTR = count;
    DMA2_Channel4->CCR = 0x1AB1;
    DMA2_Channel4->CCR |= DMA_CCR1_EN;
    
    __Set(ANALOG_ARR, arr);
    
    // Turn off the DAC output buffer - it limits the voltage range.
    DAC->CR |= DAC_CR_BOFF1;
    
    return div_round(CPUFREQ / (prescale + 1), arr + 1);
}

static cell AMX_NATIVE_CALL amx_waveout_digital(AMX *amx, const cell *params)
{
    int frequency = params[1];
    fix16_t duty = params[2];
    
    int prescale = (CPUFREQ / 65536) / frequency;
    int arr = div_round_up(CPUFREQ / (prescale + 1), frequency) - 1;
    int ccr = fix16_mul(duty, arr + 1);
    
    if (arr == 0) return 0;
    
    __Set(DIGTAL_ARR, arr);
    __Set(DIGTAL_PSC, prescale);
    __Set(DIGTAL_CCR, ccr);
    
    return div_round(CPUFREQ / (prescale + 1), arr + 1);
}

int amxinit_waveout(AMX *amx)
{
    static const AMX_NATIVE_INFO funcs[] = {
        {"waveout_analog", amx_waveout_analog},
        {"waveout_digital", amx_waveout_digital},
        {0, 0}
    };
    
    return amx_Register(amx, funcs, -1);
    
}