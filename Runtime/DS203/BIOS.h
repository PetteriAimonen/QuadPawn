/********************* (C) COPYRIGHT 2010 e-Design Co.,Ltd. ********************
 File Name : BIOS.h  
 Version   : DS203_APP Ver 2.5x                                  Author : bure
 
 Comments translated from Chinese using Google Translate by Petteri Aimonen 2011
*******************************************************************************/
#ifndef __BIOS_H
#define __BIOS_H

#include "stm32f10x.h"


//============================= Flash space allocation =================================

#define BIN_BASE                ((u32)(0x0802C000)) // Size < 68KB  
#define PRM_BASE                BIN_BASE + 68*1024  // Size =  2KB   
#define INF_BASE                BIN_BASE + 70*1024  // Size < 10KB   
#define APP4_BASE               ((u32)(0x08024000)) // Size = 32KB  
#define APP3_BASE               ((u32)(0x0801C000)) // Size = 32KB  
#define APP2_BASE               ((u32)(0x08014000)) // Size = 32KB  
#define APP1_BASE               ((u32)(0x0800C000)) // Size = 32KB  
#define SYS_BASE                ((u32)(0x08004000)) // Size = 32KB   
#define DFU_BASE                ((u32)(0x08000000)) // Size = 16KB 

//====================== Function defined in the Set Object and Value =================== ===

#define CH_A_OFFSET       0     // A channel vertical displacement Value = 0 ~ 200
#define CH_B_OFFSET       1     // B-channel vertical displacement Value = 0 ~ 200
#define BACKLIGHT         2     // Backlight Brightness Value = 0 ~ 100
#define BEEP_VOLUME       3     // Buzzer Volume Value = 0 ~ 100
#define BETTERY_DT        4     // battery voltage detection Value = 1: start
#define ADC_MODE          5     // ADC operating mode Value = 1 / 0
#define FIFO_CLR          6     // FIFO Pointer Reset Value = 1 / 0: W_PTR / R_PTR
  //#define R_PTR           0       // FIFO read address pointer is reset ( Note: Seems to be used for something else in FPGA V2.50)
  #define W_PTR           1       // FIFO write & read address pointer is reset
#define T_BASE_PSC        7     // Time Base Prescale Value = 0 ~ 65535
#define T_BASE_ARR        8     // time base divider value Value = 0 ~ 65535
#define CH_A_COUPLE       9     // A channel coupling Value = 1 / 0: AC / DC
  #define DC              0
  #define AC              1
#define CH_A_RANGE       10     // A channel input range Value = 0 ~ 5
#define CH_B_COUPLE      11     // B-channel coupling Value = 1 / 0: AC / DC
//#define DC              0
//#define AC              1
#define CH_B_RANGE       12     // B channel input range Value = 0 ~ 5
#define ANALOG_ARR       13     // analog output divider value Value = 0 ~ 65535
#define ANALOG_PTR       14     // analog output pointer Value = 0 ~ 65535
#define ANALOG_CNT       15     // number of points per cycle synthesis Value = 0 ~ 65535
#define DIGTAL_PSC       16     // pulse output prescaler Value = 0 ~ 65535
#define DIGTAL_ARR       17     // pulse output divider value Value = 0 ~ 65535
#define DIGTAL_CCR       18     // pulse output duty cycle value Value = 0 ~ 65535
#define KEY_IF_RST       19     // timer interrupt flag is reset Value = 0
#define STANDBY          20     // waiting to enter the power-down Value = 0
#define FPGA_RST         31     // FPGA Reset Value = 0

#define TRIGG_MODE       32+0  // trigger mode Value = Mode
#define V_THRESHOLD      32+1  // voltage trigger threshold Value = 0 ~ 200
#define T_THRESHOLD      32+2  // pulse width trigger time threshold Value = 0 ~ 65535
#define ADC_CTRL         32+4  // ADC work status Set Value = 1 / 0 EN / DN
#define A_POSITION       32+5  // CH_A zero point Value = 0 ~ 200
#define B_POSITION       32+6  // CH_B zero point Value = 0 ~ 200
#define REG_ADDR         32+7  // the address determines which set of registers in the FPGA the data read by the MCU

/* Input channel resolutions */
#define ADC_50mV 0
#define ADC_100mV 1
#define ADC_200mV 2
#define ADC_500mV 3
#define ADC_1V 4
#define ADC_2V 5
#define ADC_5V 6
#define ADC_10V 7
#define ADC_RANGE_COUNT 8

//==================== Set the function defined in TRIGG_MODE =====================
/*
CH_A Trigger source & kind select => 
0x00: by Negedge;   0x01: by Posedge;   0x02: by low level; 0x03: by high level
0x04: TL < Delta_T; 0x05: TL > Delta_T; 0x06: TH < Delta_T; 0x07: TH > Delta_T;

CH_B Trigger source & kind select =>
0x08: by Negedge;   0x09: by Posedge;   0x0A: by low level; 0x0B: by high level
0x0C: TL < Delta_T; 0x0D: TL > Delta_T; 0x0E: TH < Delta_T; 0x0F: TH > Delta_T;

CH_C Trigger source & kind select =>
0x10: by Negedge;   0x11: by Posedge;   0x12: by low level; 0x13: by high level
0x04: TL < Delta_T; 0x05: TL > Delta_T; 0x06: TH < Delta_T; 0x07: TH > Delta_T;

CH_D Trigger source & kind select =>
0x18: by Negedge;   0x19: by Posedge;   0x1A: by low level; 0x1B: by high level
0x1C: TL < Delta_T; 0x1D: TL > Delta_T; 0x1E: TH < Delta_T; 0x1F: TH > Delta_T;

0x20~0xFF  =>  Unconditional trigger
*/
#define UNCONDITION       0x20        // Unconditional trigger sampling

//================ Function Set the Value in the definition ADC_CTRL & STANDBY ================

#define DN            0
#define EN            1

//===================== Set the Value function defined in ADC_MODE =====================

#define SEPARATE      0    // ADC independent sampling mode
#define INTERLACE     1    // ADC Alternate Sampling mode

//========================= Get the Kind in the definition of the function ============================

#define FIFO_DIGIT             0    // 16bits FIFO digital data
#define FIFO_EMPTY             1    // FIFO empty flag: 1 = empty
#define FIFO_START             2    // FIFO start flag: 1 = start
#define FIFO_FULL              3    // FIFO full flag: 1 = Full
#define KEY_STATUS             4    // Current keys status 
#define K_ITEM_D_STATUS      0x0008    // 0 = Key push on
#define K_ITEM_S_STATUS      0x0040    // 0 = Key push on
#define KEY3_STATUS          0x0100    // 0 = Key push on
#define KEY4_STATUS          0x0200    // 0 = Key push on
#define K_INDEX_D_STATUS     0x0400    // 0 = Key push on
#define K_INDEX_I_STATUS     0x0800    // 0 = Key push on
#define K_INDEX_S_STATUS     0x1000    // 0 = Key push on
#define KEY2_STATUS          0x2000    // 0 = Key push on
#define KEY1_STATUS          0x4000    // 0 = Key push on
#define K_ITEM_I_STATUS      0x8000    // 0 = Key push on
#define ALL_KEYS 0xFF48
#define USB_POWER              5    // USB power status: 1 = Power ON
#define V_BATTERY              6    // Battery voltage (mV)
#define VERTICAL               7    // pointer to the vertical channel properties
#define HORIZONTAL             8    // pointer to the level of channel properties
#define GLOBAL                 9    // pointer to the overall properties
#define TRIGGER                10   // pointer to trigger channel properties
#define FPGA_OK                11   // FPGA configuration is successful 1 = FPGA config ok
#define CHARGE         12       // battery charge status
#define HDWVER         13       // device hardware version 
#define DFUVER         14       // DFU version number of program modules
#define SYSVER         15       // SYS version number of program modules
#define FPGAVER        16       // FPGA configuration program version number

// FPGA register addresses for Set_Param
#define FPGA_SP_TRIGGMODE    0 // Note: resets other settings to defaults
#define FPGA_SP_VTHRESHOLD   1
#define FPGA_SP_TTHRESHOLD_L 2 // Time threshold, bits 0..7
#define FPGA_SP_TTHRESHOLD_H 3 // Time threshold, bits 8..15
#define FPGA_SP_CTRLREG      4 // Control register
#define FPGA_SP_SELECT       5 // Read selector for reading trigger results
#define FPGA_SP_DEPTH_L      6 // Sample count, bits 0..7
#define FPGA_SP_DEPTH_H      7 // Sample count, bits 8..11
#define FPGA_SP_PERCNT_L     8 // Presample count, bits 0..7
#define FPGA_SP_PERCNT_H     9 // Presample count, bits 8..11
#define FPGA_SP_DELAY_1     10 // Trigger delay, bits 0..7
#define FPGA_SP_DELAY_2     11 // Trigger delay, bits 8..15
#define FPGA_SP_DELAY_3     12 // Trigger delay, bits 16..23
#define FPGA_SP_DELAY_4     13 // Trigger delay, bits 24..31

// =============================================================================

typedef struct  // hardware integrated properties 
{
  u16 LCD_X;    // screen horizontal pixels
  u16 LCD_Y;    // screen vertical pixels
  u16 Yp_Max;   // vertical scale maximum     
  u16 Xp_Max;   // maximum level stalls     
  u16 Tg_Num;   // trigger the maximum stall
  u16 Yv_Max;   // maximum vertical displacement 
  u16 Xt_Max;   // maximum horizontal displacement 
  u16 Co_Max;   // maximum coupling 
  u8  Ya_Num;   // the number of analog channels
  u8  Yd_Num;   // the number of digital channels
  u8  INSERT;   // Start application gear interpolation
  u16 KpA1;     // A channel shift compensation factor 1
  u16 KpA2;     // A channel shift compensation factor 2
  u16 KpB1;     // B channel compensation coefficient of an offset
  u16 KpB2;     // B channel compensation coefficient 2 displacement
} G_attr ;

typedef struct  // vertical channel properties 
{
  u8  STR[8];   // stall identification string
  s16 KA1;      // A channel displacement error correction factor 1
  u16 KA2;      // A channel slope error correction factor 2
  s16 KB1;      // B channel displacement error correction factor 1
  u16 KB2;      // B channel slope error correction factor 2
  u32 SCALE;    // vertical channel scale factor
} Y_attr ;

typedef struct  // horizontal channel properties 
{
  u8  STR[8];   // stall identification string
  s16 PSC;      // prescaler
  u16 ARR;      // frequency coefficient
  u16 CCR;      // duty factor
  u16 KP;       // interpolation factor
  u32 SCALE;    // horizontal channel scale factor
} X_attr ; 

typedef struct  // trigger the channel properties 
{
  u8  STR[8];   // trigger identification string
  u8  CHx;      // trigger the channel number
  u8  CMD;      // trigger control word
} T_attr ; 

extern Y_attr *Y_Attr; 
extern X_attr *X_Attr; 
extern G_attr *G_Attr; 
extern T_attr *T_Attr; 

// CPU clock frequency is 72 MHz.
#define CPUFREQ 72000000

// Font size
#define FONT_HEIGHT 14
#define FONT_WIDTH 8

//==============================================================================
//                        System function entrance
//==============================================================================
 void __CTR_HP(void);     //USB_HP_Interrupt
 void __USB_Istr(void);   //USB_LP_Interrupt
 void __USB_Init(void);                      
 
 void __LCD_Initial(void);
 void __Clear_Screen(u16 Color);
 void __Point_SCR(u16 x0, u16 y0);
 void __LCD_SetPixl(u16 Color);
 u16  __LCD_GetPixl(void);  
 u16  __Get_TAB_8x14(u8 Code, u16 Row);
 void __LCD_Set_Block(u16 x1, u16 x2, u16 y1, u16 y2);
 
 void __LCD_Copy(uc16 *pBuffer, u16  NumPixel); // Send a row data to LCD
 void __LCD_Fill(u16 *pBuffer,u16 NumPixel);    // Fill number of pixel by DMA 
 void __LCD_DMA_Ready(void);                    // Wait LCD data DMA ready
 
 void __Row_Copy(uc16 *S_Buffer,u16 *T_Buffer); // Copy one row base data to buffer
 void __Row_DMA_Ready(void);                    // Wait row base data DMA ready
 
 u32  __Read_FIFO(void);                        // Read data from FIFO & Ptr+1
 void __Display_Str(u16 x0, u16 y0, u16 Color, u8 Mode, u8 *s);                      
 
 u32  __Input_Lic(u16 x0, u8 y0);               //Return: 32Bits Licence 
 u32  __GetDev_SN(void);                        // Get 32bits Device Serial Number

 u8   __Ident(u32 Dev_ID, u32 Proj_ID, u32 Lic_No);
 
 void __Set(u8 Object, u32 Value);
 u32  __Get(u8 Object);
 
 void __Set_Param(u8 RegAddr, u8 Parameter);
 
 void __ExtFlash_PageRD(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead);
 void __ExtFlash_PageWR(u8* pBuffer, u32 WriteAddr);
 u8   __ReadDiskData(u8* pBuffer, u32 ReadAddr, u16 Lenght);
 u8   __ProgDiskPage(u8* pBuffer, u32 ProgAddr);
// u8   __ClashSt(void); 
// void __ClashClr(void); 
 
 u8   __FLASH_Erase(u32 Address);
 u8   __FLASH_Prog(u32 Address, u16 Data);
 void __FLASH_Unlock(void);
 void __FLASH_Lock(void);
 
 u8   __Chk_SYS(u32 Licence);
 u8*  __Chk_DFU(void);
 u8*  __Chk_HDW(void);

 u8 __OpenFileWr(u8* Buffer, u8* FileName, u16* Cluster, u32* pDirAddr);
 u8 __OpenFileRd(u8* Buffer, u8* FileName, u16* Cluster, u32* pDirAddr);
 u8 __ReadFileSec(u8* Buffer, u16* Cluster);
 u8 __ProgFileSec(u8* Buffer, u16* Cluster);
 u8 __CloseFile(u8* Buffer, u32 Lenght, u16* Cluster, u32* pDirAddr);
/**/

/* Some other stuff that is not exported by the BIOS */
#include <stm32f10x.h>

#define always_read(x) asm(""::"r"(x))

#define FPGA_HL_LOW() GPIOC->BRR = (1<<5);

// Seems like we need a bit of delay around the RS changes
static void LCD_DELAY()
{
    for (int i = 0; i < 10; i++)
        asm("nop");
}

#define LCD_RS_LOW()      GPIOD->BRR  = (1<<12)
#define LCD_RS_HIGH()     GPIOD->BSRR = (1<<12)
#define LCD_PORT    (*((vu16 *)(0x60000000+0x00)))
#define LCD_TYPE_ILI9327 0x02049327

static void LCD_WR_Ctrl(u16 Reg) 
{
    LCD_RS_LOW();
    LCD_PORT = Reg;        //Reg. Addr.
    LCD_DELAY();
    LCD_RS_HIGH();
}

static void LCD_WR_REG(u16 Reg, u16 Data) 
{
    LCD_RS_LOW();
    LCD_PORT = Reg;        //Reg. Addr.
    LCD_DELAY();
    LCD_RS_HIGH();
    LCD_PORT  = Data;       //Reg. Data 
    LCD_DELAY();
}

static uint32_t LCD_RD_Type() 
{ 
    uint32_t LCD_Type;
    LCD_WR_Ctrl(0xEF);
    always_read(LCD_PORT);
    LCD_Type  = (LCD_PORT&0xFF)<<24;
    LCD_Type |= (LCD_PORT&0xFF)<<16;
    LCD_Type |= (LCD_PORT&0xFF)<<8;
    LCD_Type |= (LCD_PORT&0xFF);
    return LCD_Type;
}

#endif  
/*******************************  END OF FILE  ********************************/
