/* USART Serial port */
#include "amx.h"
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
//#include "drawing.h"
#include "BIOS.h"

#include "stm32f10x.h"

#define RX_SIZE 	128
#define rx_is_empty()  (usart_w == usart_r)

uint8_t usart_r=0;//read position
uint8_t usart_w=0;//write position
uint8_t usart_cnt=0;//available bytes in data
char    usart_data[RX_SIZE];//buffer


/// IRQ handler
void USART1_IRQHandler(void) {
  usart_data[ usart_w ] = USART1->DR;
  usart_w = ( usart_w + 1) % RX_SIZE;
  usart_cnt++;
}

///Public functions-----------------------------------------------

/// Write bytes to file. Returns false if there was any error.
/// If count is -1, stops on 0 (string terminator) in src. Otherwise
/// writes exactly count bytes.
/// native bool: usart_write(const src{}, count = -1);
static cell AMX_NATIVE_CALL amx_usart_write(AMX *amx, const cell *params){
    uint8_t *ptr,cnt=0;
    amx_StrParam(amx, params[1], ptr);  
    while((*ptr && params[2]==-1)||(params[2]>0 && cnt < params[2])){//while *ptr is not NULL
      while (!(USART1->SR & USART_SR_TXE));
      USART1->DR = *ptr++;
      cnt++;
    }
    return 1;
}

/// Read raw bytes from file. Returns number of bytes read.
/// native usart_read(buf{}, maxsize = sizeof buf);
static cell AMX_NATIVE_CALL amx_usart_read(AMX *amx, const cell *params){
  char *ptr=(char*)params[1];
  uint8_t cnt=0;
  int max = ((uint8_t )params[2])*4;//max bytes is maxsize*4
  //printf("\r\nusart_read max:%d",max);
  while((usart_w != usart_r) && cnt < max){
    *ptr++=usart_data[usart_r];//get data from buffer
    usart_r=( usart_r + 1) % RX_SIZE;//new read position
    usart_cnt--;//update usart data available
    cnt++;//readed bytes
  }
  *ptr=0;

  //from f_read:
  // Swap the bytes in each cell to convert into Pawn packed string
  cell* p = (cell*)params[1];
  for (int i = 0; i < params[2]; i++, p++)
  {
      *p = __builtin_bswap32(*p);
  }

  //printf(" [%d] [%s]",cnt,(char*)params[1]);
  return cnt;//readed bytes
}

/// available bytes in rx buffer
/// native bool: usart_rxcount();
static cell AMX_NATIVE_CALL amx_usart_rxcount(AMX *amx, const cell *params){
  return usart_cnt;
}

int amxinit_usart(AMX *amx)
{
    static const AMX_NATIVE_INFO funcs[] = {
        {"usart_write", amx_usart_write},
        {"usart_read", amx_usart_read},
        {"usart_rxcount", amx_usart_rxcount},
        {0, 0}
    };
    //usart_r=usart_w=0;//empty buffer
    //usart_data[0]=0;
    return amx_Register(amx, funcs, -1);
}
