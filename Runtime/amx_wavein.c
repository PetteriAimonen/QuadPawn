/** Functions for accessing the ADC from PAWN */

#include <stdbool.h>
#include <stdint.h>
#include "amx.h"
#include "BIOS.h"
#include "ds203_io.h"
#include <stdio.h>
#include <stm32f10x.h>
#include "mathutils.h"
#include "utils.h"

extern volatile bool ABORT;

static cell ch_a_couple, ch_a_range, ch_a_offset;
static cell ch_b_couple, ch_b_range, ch_b_offset;

static cell AMX_NATIVE_CALL amx_config_chA(AMX *amx, const cell *params)
{
    // config_chA(ADCCoupling: coupling, ADCRange: range, offset = 0);
    ch_a_couple = params[1];
    ch_a_range = params[2];
    ch_a_offset = params[3];
    __Set(CH_A_COUPLE, ch_a_couple);
    __Set(CH_A_RANGE, ch_a_range);
    __Set(CH_A_OFFSET, ch_a_offset);
    
    return 0;
}

static cell AMX_NATIVE_CALL amx_config_chB(AMX *amx, const cell *params)
{
    ch_b_couple = params[1];
    ch_b_range = params[2];
    ch_b_offset = params[3];
    __Set(CH_B_COUPLE, ch_b_couple);
    __Set(CH_B_RANGE, ch_b_range);
    __Set(CH_B_OFFSET, ch_b_offset);
    
    return 0;
}

static cell AMX_NATIVE_CALL amx_getconfig_chA(AMX *amx, const cell *params)
{
    *(cell*)params[1] = ch_a_couple;
    *(cell*)params[2] = ch_a_range;
    *(cell*)params[3] = ch_a_offset;
    return 0;
}

static cell AMX_NATIVE_CALL amx_getconfig_chB(AMX *amx, const cell *params)
{
    *(cell*)params[1] = ch_b_couple;
    *(cell*)params[2] = ch_b_range;
    *(cell*)params[3] = ch_b_offset;
    return 0;
}

static cell AMX_NATIVE_CALL amx_wavein_samplerate(AMX *amx, const cell *params)
{
    // wavein_samplerate(frequency);
    int freq = params[1];
    int prescale = (CPUFREQ / 65536) / freq; // Prescale is used only for <1000Hz
    int arr = div_round_up(CPUFREQ / (prescale + 1), freq) - 1;
    
    if (arr < 0) arr = 0;
    
    __Set(T_BASE_PSC, prescale);
    __Set(T_BASE_ARR, arr);
    
    return div_round(CPUFREQ / (prescale + 1), arr + 1);
}

static bool trigger_is_unconditional = false;

static cell AMX_NATIVE_CALL amx_wavein_settrigger(AMX *amx, const cell *params)
{
    // wavein_trigger(mode, channel, threshold, pulse_time, delay, interlace);
    __Set_Param(FPGA_SP_TRIGGMODE, params[1] + params[2] * 8);
    __Set_Param(FPGA_SP_VTHRESHOLD, params[3]);
    __Set_Param(FPGA_SP_TTHRESHOLD_L, params[4] & 0xFF);
    __Set_Param(FPGA_SP_TTHRESHOLD_H, params[4] >> 8);
    __Set_Param(FPGA_SP_DELAY_1, (params[5] >> 0) & 0xFF);
    __Set_Param(FPGA_SP_DELAY_2, (params[5] >> 8) & 0xFF);
    __Set_Param(FPGA_SP_DELAY_3, (params[5] >> 16) & 0xFF);
    __Set_Param(FPGA_SP_DELAY_4, (params[5] >> 24) & 0xFF);
    
    if (params[6])
    {
        __Set_Param(FPGA_SP_CTRLREG, 3); // Interlacing
        __Set(ADC_MODE, INTERLACE);
    }
    else
    {
        __Set_Param(FPGA_SP_CTRLREG, 1);
        __Set(ADC_MODE, SEPARATE);
    }
    
    trigger_is_unconditional = (params[1] >= 0x20);
    
    return 0;
}

static cell AMX_NATIVE_CALL amx_wavein_start(AMX *amx, const cell *params)
{
    // wavein_start(bool: sync = false);

    bool sync = params[1];
    
    __Set(FIFO_CLR, W_PTR);
    
    if (sync)
    {
        // Synchronize to waveout
        if (DAC->CR & DAC_CR_EN1)
        {
            // Synchronizing to DAC is easy, as the frequency is at most 1MHz
            // or so.
            GPIOB->BSRR = 2;
            while (DMA2_Channel4->CNDTR > 10)
            {
                if (ABORT) return 0;
            }
            
            while (DMA2_Channel4->CNDTR != 1);
            
            __disable_irq();
            while (DMA2_Channel4->CNDTR != 1);
            while (DMA2_Channel4->CNDTR == 1);
            GPIOB->BRR = 2;
            __enable_irq();
        }
        else if (TIM4->CR1 & TIM_CR1_CEN)
        {
            // Synchronizing to digital output is more difficult, because it
            // may run at 12MHz or more. This uses DMA to clear the
            // CLRW pin when TIM4 overflows.
            GPIOB->BSRR = 2;
            TIM4->DIER = 0;
            TIM4->CCR2 = 0;
            
            uint32_t val = 2;
            DMA1_Channel4->CCR = 0;
            DMA1->IFCR = DMA_IFCR_CTCIF4;
            DMA1_Channel4->CNDTR = 1;
            DMA1_Channel4->CPAR = (uint32_t)&GPIOB->BRR;
            DMA1_Channel4->CMAR = (uint32_t)&val;
            DMA1_Channel4->CCR = 0x3A11;

            __sync_synchronize();
            
            TIM4->DIER = TIM_DIER_CC2DE;
            
            while (!(DMA1->ISR & DMA_ISR_TCIF4) && !ABORT);
            DMA1_Channel4->CCR = 0;
            TIM4->DIER = 0;
        }
    }
    
    if (trigger_is_unconditional || sync)
    {
        while (!__Get(FIFO_START) && !ABORT);
        
        int skip = 151; // Get rid of the presamples
        
        for (int i = 0; i < skip; i++)
        {
            while (__Get(FIFO_EMPTY) && !__Get(FIFO_FULL) && !ABORT);
            __Read_FIFO();
        }
    }
    
    return 0;
}

static cell AMX_NATIVE_CALL amx_wavein_istriggered(AMX *amx, const cell *params)
{
    return __Get(FIFO_START);
}

// Mangle 4 samples x 4 channels into 4 channels of 4 packed samples
static bool mangle_samples(uint32_t *arrays[4], int counts[4], uint32_t samples[4])
{
    char *src = (char*)samples;
    
    for (int i = 0; i < 2; i++)
    {
        if (counts[i] > 0)
        {
            char *dest = (char*)arrays[i];
            dest[3] = src[i + 0*4];
            dest[2] = src[i + 1*4];
            dest[1] = src[i + 2*4];
            dest[0] = src[i + 3*4];
            counts[i]--;
            arrays[i]++;
        }
    }
    
    {
        union {
            char c[4];
            uint32_t i;
        } dest;
        dest.c[3] = src[2 + 0*4];
        dest.c[2] = src[2 + 1*4];
        dest.c[1] = src[2 + 2*4];
        dest.c[0] = src[2 + 3*4];
        
        if (counts[2] > 0)
        {
            *arrays[2]++ = dest.i & 0x01010101;
            counts[2]--;
        }
        
        if (counts[3] > 0)
        {
            *arrays[3]++ = (dest.i >> 1) & 0x01010101;
            counts[3]--;
        }
    }
    
    return counts[0] > 0 || counts[1] > 0 || counts[2] > 0 || counts[3] > 0;
}

static cell AMX_NATIVE_CALL amx_wavein_read(AMX *amx, const cell *params)
{
    // wavein_read(chA{}, chB{}, chC{}, chD{}, countA, countB, countC, countD);
    uint32_t **arrays = (uint32_t**)&params[1]; // Array of 4
    int *counts = (int*)&params[5]; // Array of 4
    
    while (!__Get(FIFO_START) && !ABORT);
    
    // If the FIFO is already full, we don't need to do any waiting
    bool full = __Get(FIFO_FULL);
    
    uint32_t samples[4];
    
    do
    {
        if (full)
        {
            // This branch is used for fast captures (samplerate > 1kHz)
            samples[0] = __Read_FIFO();
            samples[1] = __Read_FIFO();
            samples[2] = __Read_FIFO();
            samples[3] = __Read_FIFO();
        }
        else
        {
            // This branch is for slow samplerates, so that we can read atleast
            // some samples even before the buffer is completely full.
            for (int i = 0; i < 4; i++)
            {
                while (__Get(FIFO_EMPTY) && !__Get(FIFO_FULL) && !ABORT);
                samples[i] = __Read_FIFO();
            }
            
            full = __Get(FIFO_FULL);
        }
    } while(mangle_samples(arrays, counts, samples) && !ABORT);
    
    // I don't even want to think about the reasons why this is needed.
    // It guards against FIFO accidentally receiving LCD reads, but
    // it shouldn't be necessary.
    FPGA_HL_LOW();
    
    return 0;
}

int amxinit_wavein(AMX *amx)
{
    static const AMX_NATIVE_INFO funcs[] = {
        {"config_chA", amx_config_chA},
        {"config_chB", amx_config_chB},
        {"getconfig_chA", amx_getconfig_chA},
        {"getconfig_chB", amx_getconfig_chB},
        {"wavein_samplerate", amx_wavein_samplerate},
        {"wavein_settrigger", amx_wavein_settrigger},
        {"wavein_start", amx_wavein_start},
        {"wavein_istriggered", amx_wavein_istriggered},
        {"wavein_read", amx_wavein_read},
        {0, 0}
    };
    
    return amx_Register(amx, funcs, -1);
}

int amxcleanup_wavein(AMX *amx)
{
    return 0;
}
