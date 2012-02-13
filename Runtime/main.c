#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "BIOS.h"
#include "stm32f10x.h"
#include "ds203_io.h"
#include "mathutils.h"
#include "Interrupt.h"
#include "irq.h"

#include "gpio.h"
DECLARE_GPIO(usart1_tx, GPIOA, 9);
DECLARE_GPIO(usart1_rx, GPIOA, 10);

int main(void)
{   
    __Set(BEEP_VOLUME, 0);
    
    // USART1 8N1 115200bps debug port
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
    USART1->BRR = ((72000000 / (16 * 115200)) << 4) | 1;
    USART1->CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;
    gpio_usart1_tx_mode(GPIO_AFOUT_10);
    gpio_usart1_rx_mode(GPIO_HIGHZ_INPUT);

    debugf("Here!");
    
    while (true)
    {
    }
    
    return 0;
}

