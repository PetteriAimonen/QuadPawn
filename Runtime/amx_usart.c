/* USART Serial port */
#include "amx.h"
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
//#include "drawing.h"
#include "BIOS.h"

#include "stm32f10x.h"

//write
static cell AMX_NATIVE_CALL amx_write_usart(AMX *amx, const cell *params){
    //to USART1
    char *ptr;
    amx_StrParam(amx, params[1], ptr);
  
    while(*ptr){//while *ptr is not NULL
      while (!(USART1->SR & USART_SR_TXE));
      USART1->DR = *ptr++;
    }
    return 1;//a void return may be better?
}

int amxinit_usart(AMX *amx)
{
    static const AMX_NATIVE_INFO funcs[] = {
        {"write_usart", amx_write_usart},
        {0, 0}
    };
    return amx_Register(amx, funcs, -1);
}
