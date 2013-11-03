/* FPGA configuration loading to the ICE65L04. */

/* The ICE65 docs demand a constant clock frequency for SPI configuration,
 * but for some reason the DSO Quad has the configuration pins connected
 * to normal GPIO instead of a SPI module. This is somewhat annoying, as
 * the flash reading causes occassional slowdowns.
 *
 * However, it appears that the constant clock frequency is not really
 * necessary, and the configuration seems to work ok with simple bitbanged
 * SPI implementation.
 */

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

// Address of the original FPGA image in ROM
#define FPGA_ROM_ADDR 0x0802C000

// The FPGA SPI pins are mapped on PB10-PB13.
#define SS_PIN          (1 << 10)
#define SCK_PIN         (1 << 11)
#define SI_PIN          (1 << 12)
#define SO_PIN          (1 << 13)
#define RESET_PIN       (1 << 14)
#define DONE_PIN        (1 << 15)

// A small delay for pin toggling
#define PIN_DELAY asm("nop"); asm("nop"); asm("nop"); asm("nop");

// Transmit one byte of data over SPI.
static void spi_transmit(uint8_t byte)
{
    unsigned _byte = byte;
    
    for (int j = 0; j < 8; j++)
    {
        if (_byte & 0x80)
        {
            GPIOB->BSRR = (SCK_PIN << 16) | SI_PIN;
            PIN_DELAY;
            GPIOB->BSRR = SCK_PIN | SI_PIN;
            PIN_DELAY;
        }
        else
        {
            GPIOB->BSRR = (SCK_PIN << 16) | (SI_PIN << 16);
            PIN_DELAY;
            GPIOB->BSRR = SCK_PIN | (SI_PIN << 16);
            PIN_DELAY;
        }
        
        _byte <<= 1;
    }
}

bool fpga_configure(FIL *file)
{
    // Configure SI, SCK, SS and RESET as outputs
    uint32_t orig_crh = GPIOB->CRH;
    GPIOB->CRH = (GPIOB->CRH & 0x000000FF) | 0x83833300;
    
    // Reset the FPGA
    GPIOB->BRR = RESET_PIN | SS_PIN;
    GPIOB->BSRR = SCK_PIN;
    delay_ms(2);
    GPIOB->BSRR = RESET_PIN;
    delay_ms(2);
    
    if (file == NULL)
    {
        // Load the original FPGA image from STM32 flash
        for (unsigned i = 0; i < 68088; i++)
            spi_transmit(*(uint8_t*)(FPGA_ROM_ADDR + i));
    }
    else
    {
        while (true)
        {
            unsigned bytes_read;
            uint8_t byte;
            if (f_read(file, &byte, 1, &bytes_read) != FR_OK || bytes_read != 1)
                break;
            
            spi_transmit(byte);
        }
    }
    
    // Send a few dummy clocks to finish off
    for (int i = 0; i < 500; i++)
        spi_transmit(0x00);
    
    // Check configuration status
    bool status = ((GPIOB->IDR & DONE_PIN) != 0);
    
    // Return GPIOB pins to original config
    GPIOB->BSRR = SS_PIN;
    GPIOB->CRH = orig_crh;
    
    return status;
}
