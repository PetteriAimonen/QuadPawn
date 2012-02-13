/********************* (C) COPYRIGHT 2010 e-Design Co.,Ltd. ********************
 File Name : Interrupt.c  
 Version   : DS203_APP Ver 2.3x                                  Author : bure
*******************************************************************************/
#include "Interrupt.h"
#include "BIOS.h"

vu32 vu32Tick;
vu8  Cursor_Cnt, Key_Wait_Cnt, Key_Repeat_Cnt, Key_Buffer, Cnt_mS, Cnt_20mS;
vu8  Twink, Blink;
u8   Volume=20, Light;
vu16 Delay_Cnt, Beep_mS, Key_Status_Last, Sec_Cnt, PD_Cnt;
vu32 Wait_Cnt; 

void NMIException(void)
{}

/* Some simple debug routines */
static char linebuf[64];
static int pos = 0;
static int y = 220;

static void _putc(char b)
{
    if (b == '\n' || pos == 63)
    {
        linebuf[pos] = 0;
        __Display_Str(10, y, 0xFFFF, 0, (u8*)linebuf);
        pos = 0;
        y -= 14;
    }
    else
    {
        linebuf[pos++] = b;
    }
}

static char hexnibble(int n)
{
    if (n < 10)
        return '0' + n;
    else
        return 'A' - 10 + n;
}

static void putstring(const char *p)
{
    while (*p)
    {
        _putc(*p++);
    }
}

static void puthex(const char *p, int count)
{
    while (count--)
    {
        _putc(hexnibble(*(p + count) >> 4));
        _putc(hexnibble(*(p + count) & 0x0F));
    }
}

static void **sp;
register void **PSP asm("sp");

void _HardFaultException();
void HardFaultException(void) __attribute__((noreturn, naked));
void HardFaultException()
{
    // Rescue stack pointer and program counter
    asm("mrs %0, msp" : "=r"(sp) : :);
    _HardFaultException();
}

void _HardFaultException()
{
    __Clear_Screen(0b0000000000011111);
    
    __Set(BEEP_VOLUME, 0);
    
    putstring("\n         HARDFAULT          ");

    putstring("\nSP: ");
    puthex((char*) &sp, sizeof(void*));
    putstring("  PSP: ");
    void *psp = PSP;
    puthex((char*) &psp, sizeof(void*));
    
    putstring("\nPC: ");
    puthex((char*) (sp + 6), sizeof(void*));
    putstring("  LR: ");
    puthex((char*) (sp + 5), sizeof(void*));
    putstring("\nREGS: r0 ");
    puthex((char*) (sp + 0), sizeof(void*));
    putstring(", r1 ");
    puthex((char*) (sp + 1), sizeof(void*));
    putstring("\n      r2 ");
    puthex((char*) (sp + 2), sizeof(void*));
    putstring(", r3 ");
    puthex((char*) (sp + 3), sizeof(void*));
    putstring(", r12 ");
    puthex((char*) (sp + 4), sizeof(void*));
    
    putstring("\nSCB HFSR: ");
    puthex((char*) &(SCB->HFSR), sizeof(SCB->HFSR));
    
    putstring(" CFSR: ");
    puthex((char*) &(SCB->CFSR), sizeof(SCB->CFSR));
    
    putstring("\nSTACK DUMP:\n");
    int i;
    for (i = 0; i < 32; i++)
    {
        puthex((char*) (sp + i), sizeof(void*));
        if (i % 4 == 3)
            putstring("\n");
        else
            putstring(" ");
    }
    
    putstring("\n");
    
    while (1) {}
}

void MemManageException(void)
{
  while (1) {}
}

void BusFaultException(void)
{
  while (1) {}
}
void UsageFaultException(void)
{
  while (1) {}
}

extern void my_usb_istr();

void USB_HP_CAN_TX_IRQHandler(void)
{
  __CTR_HP();
}

void USB_LP_CAN_RX0_IRQHandler(void)
{
  __USB_Istr();
}

void TIM3_IRQHandler(void)
{ 
//  static char tmp[2] = {'A', 0};
  u8 KeyCode;
  vu32Tick++;
  __Set(KEY_IF_RST, 0);                      //Clear TIM3 interrupt flag
  
  if(Delay_Cnt > 0) Delay_Cnt--;
  
  if(Cnt_mS > 0)
      Cnt_mS--;
  else {                                     //Read keys per 20mS
    Cnt_mS =20;
    if(Wait_Cnt >0)  Wait_Cnt--;
    if(Beep_mS >=20)  Beep_mS   -= 20;
    else              __Set(BEEP_VOLUME, 0); // Beep off
    if(Cnt_20mS < 50) Cnt_20mS++;
    else {                                   // Do it pre sec.
      Cnt_20mS = 0;
      __Set(BETTERY_DT, 1);                  //Battery Detect
      Sec_Cnt++;
      if(PD_Cnt > 0) PD_Cnt--;
    }
    Cursor_Cnt++;
    if(Cursor_Cnt >= 12) {                   //12*20mS=240mS
//	  __Display_Str(0, 0, 0x8888, 0, tmp);
//	  if (++tmp[0] > 'Z')
//		tmp[0] = 'A';

      Cursor_Cnt=0;
      Twink=!Twink;
      Blink = 1;
    }
    if(Key_Wait_Cnt)    Key_Wait_Cnt--;
    if(Key_Repeat_Cnt)  Key_Repeat_Cnt--;
    KeyCode=0;//Read_Keys();
    if(KeyCode !=0) {
      Key_Buffer = KeyCode;
      //__Set(BEEP_VOLUME, 5*(Title[VOLUME][CLASS].Value-1));// Volume
      Beep_mS = 60; 
    }
  }
}
