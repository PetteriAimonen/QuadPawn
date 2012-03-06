/********************* (C) COPYRIGHT 2010 e-Design Co.,Ltd. ********************
 File Name : Interrupt.c  
 Version   : DS203_APP Ver 2.3x                                  Author : bure
*******************************************************************************/
#include "Interrupt.h"
#include "BIOS.h"
#include <stdint.h>

void NMIException(void)
{}

void USB_HP_CAN_TX_IRQHandler(void)
{
  __CTR_HP();
}

void USB_LP_CAN_RX0_IRQHandler(void)
{
  __USB_Istr();
}
