/* Pawn native functions corresponding to display.inc.
 * These implement LCD access from the virtual machine.
 */

#include <stdbool.h>
#include <stdint.h>
#include "BIOS.h"
#include "ds203_io.h"
#include "amx.h"
#include "stm32f10x.h"
#include "mathutils.h"
#include "drawing.h"

static cell AMX_NATIVE_CALL amx_draw_text(AMX *amx, const cell *params)
{
    // draw_text(const str[], x, y, foreground, background, center);
    char *text;
    amx_StrParam(amx, params[1], text);
    
    draw_text(text, params[2], params[3], params[4], params[5], params[6]);    
    
    return 0;
}

static cell AMX_NATIVE_CALL amx_draw_flowtext(AMX *amx, const cell *params)
{
    char *text;
    amx_StrParam(amx, params[1], text);
    
    draw_flowtext(text, params[2], params[3], params[4], params[5], params[6], params[7], params[8]);
    
    return 0;
}


static cell AMX_NATIVE_CALL amx_lcd_type(AMX *amx, const cell *params)
{
    return LCD_RD_Type();
}

static cell AMX_NATIVE_CALL amx_fill_rectangle(AMX *amx, const cell *params)
{
    // fill_rectangle(x, y, w, h, Color: color);
    fill_rectangle(params[1], params[2], params[3], params[4], params[5]);
    return 0;
}

static cell AMX_NATIVE_CALL amx_putpixel(AMX *amx, const cell *params)
{
    // putpixel(x, y, color);
    __Point_SCR(params[1], params[2]);
    __LCD_SetPixl(params[3]);
    
    return 0;
}

static cell AMX_NATIVE_CALL amx_getpixel(AMX *amx, const cell *params)
{
    // getpixel(x, y)
    __Point_SCR(params[1], params[2]);
    return __LCD_GetPixl();
}

static cell AMX_NATIVE_CALL amx_blend(AMX *amx, const cell *params)
{
    int fg = params[1];
    int bg = params[2];
    fix16_t opacity = params[3];
    
    return blend(fg, bg, opacity >> 8);
}

static cell AMX_NATIVE_CALL amx_putcolumn(AMX *amx, const cell *params)
{
    // putcolumn(x, y, const pixels[], count, wait);
    __Point_SCR(params[1], params[2]);
    
    cell *pixels = (cell*)params[3];
    int count = params[4];
    
    // Routine copied from SYS1.50 source and modified to do
    // 32 -> 16 bit mapping on the fly.
    DMA2_Channel1->CCR = 0x5990;
    DMA2_Channel1->CMAR = (uint32_t)pixels;
    DMA2_Channel1->CNDTR = count;
    DMA2_Channel1->CCR = 0x5991;
    
    if (params[4])
    {
        __LCD_DMA_Ready();
    }
    
    return 0;
}

static cell AMX_NATIVE_CALL amx_getcolumn(AMX *amx, const cell *params)
{
    // getcolumn(x, y, pixels[], count);
    
    int x = params[1];
    int y = params[2];
    cell *pixels = (cell*)params[3];
    int count = params[4];
    
    // Seems like DSO Quad may use two different kinds of LCD's, with
    // slightly different command sets..
    if (LCD_RD_Type() == LCD_TYPE_ILI9327)
    {
        __Point_SCR(x, y);
        LCD_WR_Ctrl(0x2E);
        
        // Dummy read
        always_read(LCD_PORT);
        
        // Use DMA to do the transfer
        DMA2_Channel1->CCR = 0x5980;
        DMA2_Channel1->CMAR = (uint32_t)pixels;
        DMA2_Channel1->CNDTR = count;
        DMA2_Channel1->CCR = 0x5981;
        
        __LCD_DMA_Ready();
    
        LCD_WR_Ctrl(0x2C);
    }
    else
    {
        // I haven't been able to test this code path.
        __LCD_DMA_Ready();
        // The R61509V doesn't automatically increment the address
        // when reading, which slows this down a bit...
        LCD_WR_REG(0x0201, x);

        while (count--)
        {
            LCD_WR_REG(0x0200, y++);
            LCD_WR_Ctrl(0x0202);
            always_read(LCD_PORT);
            *pixels++ = LCD_PORT;
        }
    }
    
    return 0;
}

static cell AMX_NATIVE_CALL amx_drawline_aa(AMX *amx, const cell *params)
{
    // drawline(x1, y1, x2, y2, color);
    drawline_aa(params[1], params[2], params[3], params[4], params[5]);
    return 0;
}

static cell AMX_NATIVE_CALL amx_drawline(AMX *amx, const cell *params)
{
    // drawline(x1, y1, x2, y2, color, dots)
    drawline(params[1], params[2], params[3], params[4], params[5], params[6]);
    return 0;
}    

static cell AMX_NATIVE_CALL amx_draw_rectangle(AMX *amx, const cell *params)
{
    draw_rectangle(params[1], params[2], params[3], params[4], params[5], params[6]);
    return 0;
}    

static cell AMX_NATIVE_CALL amx_draw_bitmap(AMX *amx, const cell *params)
{
    // draw_bitmap(const bitmap[], x, y, Color: color, count = sizeof bitmap, center);
    draw_bitmap((uint32_t*)params[1], params[2], params[3], params[4], params[5], params[6]);
    return 0;
}

int amxinit_display(AMX *amx)
{
    static const AMX_NATIVE_INFO funcs[] = {
        {"draw_text", amx_draw_text},
        {"draw_flowtext", amx_draw_flowtext},
        {"lcd_type", amx_lcd_type},
        {"fill_rectangle", amx_fill_rectangle},
        {"putpixel", amx_putpixel},
        {"getpixel", amx_getpixel},
        {"blend", amx_blend},
        {"putcolumn", amx_putcolumn},
        {"getcolumn", amx_getcolumn},
        {"drawline_aa", amx_drawline_aa},
        {"drawline", amx_drawline},
        {"draw_rectangle", amx_draw_rectangle},
        {"draw_bitmap", amx_draw_bitmap},
        {0, 0}
    };
    
    return amx_Register(amx, funcs, -1);
}
