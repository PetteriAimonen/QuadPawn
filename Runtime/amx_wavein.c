/** Functions for accessing the ADC from PAWN */

#include <stdbool.h>
#include <stdint.h>
#include "amx.h"
#include "BIOS.h"
#include "ds203_io.h"
#include <stdio.h>
#include <stm32f10x.h>
#include "mathutils.h"

static cell AMX_NATIVE_CALL amx_config_chA(AMX *amx, const cell *params)
{
    // config_chA(ADCCoupling: coupling, ADCRange: range, offset = 0);
    __Set(CH_A_COUPLE, params[1]);
    __Set(CH_A_RANGE, params[2]);
    __Set(CH_A_OFFSET, params[3]);
    
    return 0;
}

static cell AMX_NATIVE_CALL amx_config_chB(AMX *amx, const cell *params)
{
    __Set(CH_B_COUPLE, params[1]);
    __Set(CH_B_RANGE, params[2]);
    __Set(CH_B_OFFSET, params[3]);
    
    return 0;
}

static cell AMX_NATIVE_CALL amx_wavein_samplerate(AMX *amx, const cell *params)
{
    // wavein_samplerate(frequency);
    int freq = params[1];
    int prescale = (CPUFREQ / 65536) / freq; // Prescale is used only for <1000Hz
    int arr = div_round_up(CPUFREQ / (prescale + 1), freq) - 1;
    
    __Set(T_BASE_PSC, prescale);
    __Set(T_BASE_ARR, arr);
    
    return div_round(CPUFREQ / (prescale + 1), arr + 1);
}

static bool trigger_is_unconditional = false;

static cell AMX_NATIVE_CALL amx_wavein_settrigger(AMX *amx, const cell *params)
{
    // wavein_trigger(mode, channel, threshold, pulse_time, delay, depth);
    __Set_Param(FPGA_SP_TRIGGMODE, params[1] + params[2] * 8);
    __Set_Param(FPGA_SP_VTHRESHOLD, params[3]);
    __Set_Param(FPGA_SP_TTHRESHOLD_L, params[4] & 0xFF);
    __Set_Param(FPGA_SP_TTHRESHOLD_H, params[4] >> 8);
    __Set_Param(FPGA_SP_DELAY_1, (params[5] >> 0) & 0xFF);
    __Set_Param(FPGA_SP_DELAY_2, (params[5] >> 8) & 0xFF);
    __Set_Param(FPGA_SP_DELAY_3, (params[5] >> 16) & 0xFF);
    __Set_Param(FPGA_SP_DELAY_4, (params[5] >> 24) & 0xFF);
    
    trigger_is_unconditional = (params[1] >= 0x20);
    
    return 0;
}

static cell AMX_NATIVE_CALL amx_wavein_start(AMX *amx, const cell *params)
{
    // wavein_start(bool: sync = false);

    __Set(FIFO_CLR, W_PTR);
    
    if (trigger_is_unconditional)
    {
        while (!__Get(FIFO_START));
        // Get rid of the presamples
        for (int i = 0; i < 150; i++)
        {
            while (__Get(FIFO_EMPTY) && !__Get(FIFO_FULL));
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
#define MANGLE_SAMPLES(CH, shift, mask) \
    if (count##CH) \
    { \
        *ch##CH++ = (((values[0] >> shift) & mask) << 24) \
                    | (((values[1] >> shift) & mask) << 16) \
                    | (((values[2] >> shift) & mask) << 8) \
                    | (((values[3] >> shift) & mask) << 0); \
        count##CH--; \
    }


static cell AMX_NATIVE_CALL amx_wavein_read(AMX *amx, const cell *params)
{
    // wavein_read(chA{}, chB{}, chC{}, chD{}, countA, countB, countC, countD);
    uint32_t *chA = (uint32_t*)params[1];
    uint32_t *chB = (uint32_t*)params[2];
    uint32_t *chC = (uint32_t*)params[3];
    uint32_t *chD = (uint32_t*)params[4];
    int countA = params[5];
    int countB = params[6];
    int countC = params[7];
    int countD = params[8];
    
    printf("Start\n");
    while (!__Get(FIFO_START));
    printf("Started\n");
    
    // If the FIFO is already full, we don't need to do any waiting
    bool full = __Get(FIFO_FULL);
    printf("Full: %d\n", full);
    
    while (countA || countB || countC || countD)
    {
        printf("Samples left: %d\n", countA);
        uint32_t values[4];
        
        if (full)
        {
            // This branch is used for fast captures (samplerate > 1kHz)
            values[0] = __Read_FIFO();
            values[1] = __Read_FIFO();
            values[2] = __Read_FIFO();
            values[3] = __Read_FIFO();
        }
        else
        {
            // This branch is for slow samplerates, so that we can read atleast
            // some samples even before the buffer is completely full.
            for (int i = 0; i < 4; i++)
            {
                while (__Get(FIFO_EMPTY) && !__Get(FIFO_FULL));
                values[i] = __Read_FIFO();
            }
            
            full = __Get(FIFO_FULL);
        }
        
        MANGLE_SAMPLES(A, 0, 0xFF);
        MANGLE_SAMPLES(B, 8, 0xFF);
        MANGLE_SAMPLES(C, 16, 0x01);
        MANGLE_SAMPLES(D, 17, 0x01);
    }
    
    return 0;
}

/* Real time capture stuff */

// GPIOC->BSRR values to toggle GPIOC5
static const uint32_t hl_set[2] = {1 << (16 + 5), 1 << 5};
static int fifo_halfsize;
static uint32_t *fifo;
static int fifo_read_pos;

static cell AMX_NATIVE_CALL amx_wavein_realtime_start(AMX *amx, const cell *params)
{
    // wavein_realtime_start(fifo[], samplerate, count = sizeof fifo);
    fifo_halfsize = (params[3] / 2) & 0xFFFFC; // Have to be divisible by 4
    
    // We want the ADC to sample as fast as possible, because there is some
    // 6 cycle lag between the ADC & digital channels in realtime mode.
    __Set(TRIGG_MODE, UNCONDITION);
    __Set(T_BASE_ARR, 1);
    
    // Calculate the parameters for TIM1. We want 2 cycles per each sample, to
    // read the two parts of the signal.
    int freq = params[2] * 2;
    int prescale = (CPUFREQ / 65536) / freq; // Prescale is used only for <1000Hz
    int arr = div_round_up(CPUFREQ / (prescale + 1), freq) - 1;
    
    TIM1->CR1 = 0; // Turn the timer off until we are ready
    TIM1->CR2 = 0;
    TIM1->CNT = 0;
    TIM1->SR = 0;
    TIM1->PSC = prescale;
    TIM1->ARR = arr;
    TIM1->CCMR1 = 0;
    TIM1->CCMR2 = 0;
    TIM1->DIER = TIM_DIER_CC2DE | TIM_DIER_CC4DE;
    TIM1->CCR1 = 0;
    TIM1->CCR2 = 0;
    TIM1->CCR4 = 2;
    
    // DMA Channel 3: Copy data from hl_set to GPIOC->BSRR
    DMA1_Channel3->CCR = 0;
    DMA1_Channel3->CNDTR = 2;
    DMA1_Channel3->CPAR = (uint32_t)&GPIOC->BSRR;
    DMA1_Channel3->CMAR = (uint32_t)hl_set;
    DMA1_Channel3->CCR = 0x3AB1;
    GPIOC->BSRR = hl_set[1];
    
    // DMA Channel 4: Copy data from FPGA to fifo
    DMA1_Channel4->CCR = 0;
    DMA1_Channel4->CNDTR = fifo_halfsize * 4; // two transfers per cell
    DMA1_Channel4->CPAR = 0x64000000; // FPGA memory-mapped address
    DMA1_Channel4->CMAR = params[1];
    DMA1_Channel4->CCR = 0x35A1;
    DMA1->IFCR = DMA_IFCR_CTCIF4;
    
    fifo = (uint32_t*)params[1];
    fifo_read_pos = 0;
    
    // Now, lets go!
    TIM1->CR1 |= TIM_CR1_CEN;
    
    return div_round(CPUFREQ / (prescale + 1), arr + 1);
}

static cell AMX_NATIVE_CALL amx_wavein_realtime_stop(AMX *amx, const cell *params)
{
    TIM1->DIER = 0;
    DMA1_Channel3->CCR = 0;
    DMA1_Channel4->CCR = 0;
    fifo_read_pos = -1;
    
    return 0;
}
    
static cell AMX_NATIVE_CALL amx_wavein_realtime_read(AMX *amx, const cell *params)
{
    if (fifo_read_pos < 0) return false;
    
    // wavein_realtime_read(chA{}, chB{}, chC{}, chD{}, countA, countB, countC, countD);
    uint32_t *chA = (uint32_t*)params[1];
    uint32_t *chB = (uint32_t*)params[2];
    uint32_t *chC = (uint32_t*)params[3];
    uint32_t *chD = (uint32_t*)params[4];
    int countA = params[5];
    int countB = params[6];
    int countC = params[7];
    int countD = params[8];
    
    int write_pos = fifo_halfsize * 2 - DMA1_Channel4->CNDTR / 2;
    
    if (write_pos > fifo_read_pos && (DMA1->ISR & DMA_ISR_TCIF4))
    {
        // Overflow
        amx_wavein_realtime_stop(amx, params);
        return false;
    }
    
    while (countA || countB || countC || countD)
    {
        while (fifo_read_pos <= write_pos && fifo_read_pos + 4 >= write_pos) {
            // Wait for new samples
            write_pos = fifo_halfsize * 2 - DMA1_Channel4->CNDTR / 2;
        }
        
        uint32_t* values = fifo + fifo_read_pos;
        
        MANGLE_SAMPLES(A, 0, 0xFF);
        MANGLE_SAMPLES(B, 8, 0xFF);
        MANGLE_SAMPLES(C, 16, 0x01);
        MANGLE_SAMPLES(D, 17, 0x01);
        
        fifo_read_pos = (fifo_read_pos + 4) % (2 * fifo_halfsize);
        if (fifo_read_pos == 0)
            DMA1->IFCR = DMA_IFCR_CTCIF4;
    }
    
    return true;
}

int amxinit_wavein(AMX *amx)
{
    static const AMX_NATIVE_INFO funcs[] = {
        {"config_chA", amx_config_chA},
        {"config_chB", amx_config_chB},
        {"wavein_samplerate", amx_wavein_samplerate},
        {"wavein_settrigger", amx_wavein_settrigger},
        {"wavein_start", amx_wavein_start},
        {"wavein_istriggered", amx_wavein_istriggered},
        {"wavein_read", amx_wavein_read},
        {"wavein_realtime_start", amx_wavein_realtime_start},
        {"wavein_realtime_read", amx_wavein_realtime_read},
        {"wavein_realtime_stop", amx_wavein_realtime_stop},
        {0, 0}
    };
    
    return amx_Register(amx, funcs, -1);
}

int amxcleanup_wavein(AMX *amx)
{
    amx_wavein_realtime_stop(NULL, NULL);
    
    return 0;
}
