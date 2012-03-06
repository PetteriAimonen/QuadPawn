#include "stm32f10x.h"
#include "Interrupt.h"
#include "BIOS.h"
#include <stdlib.h>

typedef void( *const intfunc )( void );

extern unsigned long _etext;
extern unsigned long _sidata;
extern unsigned long _sdata;
extern unsigned long _edata;
extern unsigned long _sbss;
extern unsigned long _ebss;
extern unsigned long _estack;

void Reset_Handler(void) __attribute__((__interrupt__));
void __Init_Data(void);
void Default_Handler(void);

extern int main(void);
extern void __libc_init_array(void);

__attribute__ ((section(".isr_vectors")))
void (* const g_pfnVectors[])(void) = {
    (intfunc)((unsigned long)&_estack), /* The stack pointer after relocation */
    Reset_Handler, NMIException, HardFaultException,
    MemManageException, BusFaultException, UsageFaultException, 0, 0,
    0, 0, SVCHandler, DebugMonitor, 0, PendSVC, SysTickHandler,
    WWDG_IRQHandler, PVD_IRQHandler, TAMPER_IRQHandler, RTC_IRQHandler,
    FLASH_IRQHandler, RCC_IRQHandler, EXTI0_IRQHandler,
    EXTI1_IRQHandler, EXTI2_IRQHandler, EXTI3_IRQHandler,
    EXTI4_IRQHandler, DMA1_Channel1_IRQHandler,
    DMA1_Channel2_IRQHandler, DMA1_Channel3_IRQHandler,
    DMA1_Channel4_IRQHandler, DMA1_Channel5_IRQHandler,
    DMA1_Channel6_IRQHandler, DMA1_Channel7_IRQHandler,
    ADC1_2_IRQHandler, USB_HP_CAN_TX_IRQHandler,
    USB_LP_CAN_RX0_IRQHandler, CAN_RX1_IRQHandler, CAN_SCE_IRQHandler,
    EXTI9_5_IRQHandler, TIM1_BRK_IRQHandler, TIM1_UP_IRQHandler,
    TIM1_TRG_COM_IRQHandler, TIM1_CC_IRQHandler, TIM2_IRQHandler,
    TIM3_IRQHandler, TIM4_IRQHandler, I2C1_EV_IRQHandler,
    I2C1_ER_IRQHandler, I2C2_EV_IRQHandler, I2C2_ER_IRQHandler,
    SPI1_IRQHandler, SPI2_IRQHandler, USART1_IRQHandler,
    USART2_IRQHandler, USART3_IRQHandler, EXTI15_10_IRQHandler,
    RTCAlarm_IRQHandler, USBWakeUp_IRQHandler, TIM8_BRK_IRQHandler,
    TIM8_UP_IRQHandler, TIM8_TRG_COM_IRQHandler, TIM8_CC_IRQHandler,
    ADC3_IRQHandler, FSMC_IRQHandler, SDIO_IRQHandler, TIM5_IRQHandler,
    SPI3_IRQHandler, UART4_IRQHandler, UART5_IRQHandler,
    TIM6_IRQHandler, TIM7_IRQHandler, DMA2_Channel1_IRQHandler,
    DMA2_Channel2_IRQHandler, DMA2_Channel3_IRQHandler,
    DMA2_Channel4_5_IRQHandler
};
#include "../ds203_io.h"
void __Init_Data(void) {
    
    unsigned long *src, *dst;
    /* copy the data segment into ram */
    src = &_sidata;
    dst = &_sdata;
    if (src != dst)
        while(dst < &_edata)
            *(dst++) = *(src++);

    /* zero the bss segment */
    dst = &_sbss;
    while(dst < &_ebss)
        *(dst++) = 0;
    
    /* give rest of the memory to malloc */
//     void *block_start = &_ebss;
//     void *block_end = (void*)&_estack - 4096; // Reserve 4kB for stack
//     add_malloc_block(block_start, block_end - block_start);
}

register void *stack_pointer asm("sp");
void Reset_Handler() __attribute__((noreturn, naked));
void Reset_Handler(void) {
    
    /* Initialize data and bss */
    stack_pointer = &_estack;
    SCB->VTOR = (u32)g_pfnVectors;
    __Init_Data();
    __USB_Init();
    
    main();
    while(1) {}
}
