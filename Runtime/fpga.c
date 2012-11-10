#include "fpga.h"
#include <stdint.h>
#include <stm32f10x.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <ff.h>
#include "buttons.h"
#include "ds203_io.h"
#include "BIOS.h"
#include "irq.h"

// Block size to load from file, 512 gives best performance
#define BLOCK_SIZE 512

// Word fifo size, must be divisible by 32
#define WORD_FIFO_SIZE 64

// Address of the original FPGA image in ROM
#define FPGA_ROM_ADDR 0x0802C000

// The FPGA SPI pins are mapped on PB10-PB13.
#define SS_PIN          (1 << 10)
#define SCK_PIN         (1 << 11)
#define SI_PIN          (1 << 12)
#define SO_PIN          (1 << 13)
#define RESET_PIN       (1 << 14)
#define DONE_PIN        (1 << 15)

static uint8_t* volatile next_block;
static uint8_t *current_block;
static unsigned byte_index;
static uint32_t *word_fifo; // Bytes decoded as BSRR words

static void fill_fifo(uint32_t *dest, uint8_t byte)
{
    unsigned _byte = byte;
    
    for (int j = 0; j < 8; j++)
    {
        if (_byte & 0x80)
        {
            *dest++ = (SCK_PIN << 16) | SI_PIN;
            *dest++ = SCK_PIN | SI_PIN;
        }
        else
        {
            *dest++ = (SCK_PIN << 16) | (SI_PIN << 16);
            *dest++ = SCK_PIN | (SI_PIN << 16);
        }
        
        _byte <<= 1;
    }
}

static uint8_t get_block_byte()
{
    if (current_block == 0)
    {
        current_block = next_block;
        next_block = NULL;
        byte_index = 0;
        
        if (current_block == NULL)
        {
            crash_with_message("Oh noes: FPGA config byte fifo underflow",
                __builtin_return_address(0)
            );
        }
    }
    
    uint8_t result = current_block[byte_index];
    byte_index++;
    
    if (byte_index == BLOCK_SIZE)
        current_block = NULL;
    
    return result;
}

void __irq__ DMA1_Channel4_IRQHandler()
{
    if (DMA1->ISR & DMA_ISR_TEIF4)
    {
        crash_with_message("Oh noes: DMA channel 4 transfer error!",
            __builtin_return_address(0)
        );
    }
    else if (DMA1->ISR & DMA_ISR_HTIF4)
    {
        for (unsigned i = 0; i < WORD_FIFO_SIZE / 2; i += 16)
            fill_fifo(word_fifo + i, get_block_byte());
        
        DMA1->IFCR = DMA_IFCR_CHTIF4;
        if (DMA1->ISR & DMA_ISR_TCIF4)
        {
            crash_with_message("Oh noes: FPGA config word fifo underflow in HTIF", __builtin_return_address(0));
        }
    }
    else if (DMA1->ISR & DMA_ISR_TCIF4)
    {
        for (unsigned i = 0; i < WORD_FIFO_SIZE / 2; i += 16)
            fill_fifo(word_fifo + i + WORD_FIFO_SIZE / 2, get_block_byte());
        
        DMA1->IFCR = DMA_IFCR_CTCIF4;
        if (DMA1->ISR & DMA_ISR_HTIF4)
        {
            crash_with_message("Oh noes: FPGA config word fifo underflow in TCIF", __builtin_return_address(0));
        }
    }
}

static bool load_block(FIL *file, uint8_t *dest, int *position)
{
    if (file == NULL)
    {
        if (*position > 68088)
            return false;
        
        memcpy(dest, (uint8_t*)(FPGA_ROM_ADDR + *position), BLOCK_SIZE);
    }
    else
    {
        unsigned bytes_read;
        if (f_read(file, dest, BLOCK_SIZE, &bytes_read) != FR_OK
            || bytes_read != BLOCK_SIZE)
        {
            return false;
        }
    }
    
    *position += BLOCK_SIZE;
    return true;
}

bool fpga_configure(FIL *file, uint8_t buffer[1024])
{
    /* The ICE65 requires constant clock frequency for SPI configuration,
     * but for some reason the DSO Quad has the configuration pins connected
     * to normal GPIO instead of a SPI module. This code uses DMA1 Channel4
     * to clock out data at a constant rate, while loading it from the
     * filesystem in the meantime.
     */
    
    uint32_t word_fifo_mem[WORD_FIFO_SIZE];
    word_fifo = word_fifo_mem;
    
    uint8_t *block1 = buffer;
    uint8_t *block2 = buffer + BLOCK_SIZE;
    
    int file_position = 0;
    next_block = current_block = 0;
    byte_index = 0;
    
    // Preload the two first blocks
    if (!load_block(file, block1, &file_position) ||
        !load_block(file, block2, &file_position))
        return false;
    
    next_block = block1;
    
    // Preload the word fifo
    for (unsigned i = 0; i < WORD_FIFO_SIZE; i += 16)
        fill_fifo(word_fifo + i, get_block_byte());
    
    next_block = block2;
    
    // Configure SI, SCK, SS and RESET as outputs
    uint32_t orig_crh = GPIOB->CRH;
    GPIOB->CRH = (GPIOB->CRH & 0x000000FF) | 0x83833300;
    
    // Reset the FPGA
    GPIOB->BRR = RESET_PIN | SS_PIN;
    GPIOB->BSRR = SCK_PIN;
    delay_ms(2);
    GPIOB->BSRR = RESET_PIN;
    delay_ms(2);
    
    // Configure TIM1 for 2 MHz time base
    TIM1->CR1 = 0;
    TIM1->CR2 = 0;
    TIM1->CNT = 0;
    TIM1->SR = 0;
    TIM1->PSC = 0;
    TIM1->ARR = 35;
    TIM1->CCMR2 = 0;
    TIM1->DIER = TIM_DIER_CC4DE;
    TIM1->CCR2 = 0;
    
    // Configure DMA1 Ch 4 to write to GPIOB->BSRR
    DMA1_Channel4->CCR = 0;
    DMA1_Channel4->CNDTR = WORD_FIFO_SIZE;
    DMA1_Channel4->CPAR = (uint32_t)&GPIOB->BSRR;
    DMA1_Channel4->CMAR = (uint32_t)&word_fifo_mem;
    DMA1_Channel4->CCR = 0x3ABF;
    
    // Clear any pending interrupts for ch 4
    DMA1->IFCR = 0x0000F000;
    
    // Enable ch 4 interrupt
    NVIC_EnableIRQ(DMA1_Channel4_IRQn);
    NVIC_SetPriority(DMA1_Channel4_IRQn, 0); // Highest priority
    
    // Now, lets go!
    TIM1->CR1 |= TIM_CR1_CEN;
    
    while (true)
    {
        while (next_block);
        if (!load_block(file, block1, &file_position))
            break;
        next_block = block1;
        
        while (next_block);
        if (!load_block(file, block2, &file_position))
            break;
        next_block = block2;
    }
    
    // Send a few dummy clocks to finish off
    while (next_block);
    next_block = block1;
    
    // Wait for last write to finish
    while (next_block);
    next_block = block1;
    while (next_block);
    
    bool status = ((GPIOB->IDR & DONE_PIN) != 0);
    
    GPIOB->BSRR = SS_PIN;
    
    // Return GPIOB pins original config
    GPIOB->CRH = orig_crh;
    
    // Turn off DMA
    NVIC_DisableIRQ(DMA1_Channel4_IRQn);
    DMA1_Channel4->CCR = 0;
    
    // Restore TIM1 settings
    TIM1->CR1 = 0;
    TIM1->CCR1 = 18;
    TIM1->CCER = 0x0003;
    TIM1->CCMR1 = 0x0078;
    TIM1->CR1 = 0x0085;
    
    return status;
}