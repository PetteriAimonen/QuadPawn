#include "drawing.h"
#include "BIOS.h"
#include <stdbool.h>
#include "mathutils.h"
#include <string.h>

/* Transparent background text printing */

static void put_textcol(int x, int y, int column, int fg, int bg)
{
    __Point_SCR(x, y);
    for (int i = 0; i < FONT_HEIGHT; i++)
    {
        if (column & 4)
        {
            __LCD_SetPixl(fg);
        }
        else if (bg >= 0)
        {
            __LCD_SetPixl(bg);
        }
        else
        {
            // Transparent background, skip pixel
            __Point_SCR(x, y + i + 1);
        }
        
        column >>= 1;
    }
}

void draw_text(const char *text, int x, int y, int fg, int bg, bool center)
{
    if (center)
        x -= strlen(text) * FONT_WIDTH / 2;
    if (x < 0) x = 0;
    
    // Add some padding in the front
    put_textcol(x, y, 0, fg, bg);
    x++;
    
    while (*text)
    {
        for (int i = 0; i < FONT_WIDTH; i++, x++)
        {
            uint16_t column;
            
            if (*text == ' ')
                column = 0;
            else
                column = __Get_TAB_8x14(*text, i);
            
            put_textcol(x, y, column, fg, bg);
        }
        
        text++;
    }
}

/* Word-wrapping text drawing */

int draw_flowtext(const char *text, int x, int y, int w, int h, int fg, int bg, bool center)
{
    char buffer[60];
    int chars_w = w / FONT_WIDTH;
    
    if (chars_w > sizeof(buffer) - 1)
        chars_w = sizeof(buffer) - 1;
    
    if (center)
        x += w / 2;
    
    int lines = 0;
    while (*text && h >= FONT_HEIGHT)
    {
        // Find a proper wrap point:
        // 1) First newline character
        // 2) End of string
        // 3) Last space character
        int wrap = chars_w;
        for (int i = 0; i < chars_w; i++)
        {
            if (text[i] == 0)
            {
                wrap = i;
                break;
            }
            else if (text[i] == '\n')
            {
                wrap = i;
                break;
            }
            else if (text[i] == ' ')
            {
                wrap = i;
            }
        }
        
        // Write out the text until the wrap point
        memcpy(buffer, text, wrap);
        buffer[wrap] = 0;
        
        h -= FONT_HEIGHT;
        draw_text(buffer, x, y + h, fg, bg, center);
        
        // Prepare for the next line
        text += wrap;
        if (*text == '\n' || *text == ' ') text++;
        
        lines++;
    }
    
    return lines;
}

/* Antialiased line drawing */

// Alpha-blend two colors together. Alpha is 0 to 255.
// The ratios have been biased a bit to make the result look
// better on a cheap TFT.
int blend(int fg, int bg, int alpha)
{
    int fg_per_2 = (fg & 0xF7DE) >> 1;
    int fg_per_4 = (fg & 0xE79C) >> 2;
    int fg_per_8 = (fg & 0xC718) >> 3;
    
    int bg_per_2 = (bg & 0xF7DE) >> 1;
    int bg_per_4 = (bg & 0xE79C) >> 2;
    int bg_per_8 = (bg & 0xC718) >> 3;
    
    if (alpha > 224)
        return fg; // 100% blend
    else if (alpha > 192)
        return (fg - fg_per_8 + bg_per_8); // 88% blend
    else if (alpha > 128)
        return (fg - fg_per_4 + bg_per_4); // 75% blend
    else if (alpha > 64)
        return (fg_per_2 + bg_per_2); // 50% blend
    else if (alpha > 32)
        return (fg_per_4 + bg - bg_per_4); // 25% blend
    else
        return bg; // 0% blend
}

// Draws antialiased lines
// Xiaolin Wu's algorithm, using x/256 fixed point values
void drawline_aa(fix16_t fx1, fix16_t fy1, fix16_t fx2, fix16_t fy2, int color)
{
    bool reverse_xy = false;
    
    void swap(int *x, int *y) {
        int temp = *x;
        *x = *y;
        *y = temp;
    }
    
    // plot the pixel at (x, y) with brightness c
    void plot(int x, int y, int c) {
        if (reverse_xy)
            swap(&x, &y);
        
        __Point_SCR(x >> 8, y >> 8);
        u16 oldcolor = __LCD_GetPixl();
        __LCD_SetPixl(blend(color, oldcolor, c));
    }
    
    // Integer part of x
    int ipart(int x) {
        return x & (~0xFF);
    }
    
    int round(int x) {
        return ipart(x + 128);
    }
    
    // Fractional part of x
    int fpart(int x) {
        return x & 0xFF;
    }
    
    // Remaining fractional part of x
    int rfpart(int x) {
        return 256 - fpart(x);
    }
    
    int x1 = fx1 >> 8;
    int x2 = fx2 >> 8;
    int y1 = fy1 >> 8;
    int y2 = fy2 >> 8;

    int dx = x2 - x1;
    int dy = y2 - y1;
    if (abs(dx) < abs(dy))
    {
        swap(&x1, &y1);
        swap(&x2, &y2);
        swap(&dx, &dy);
        reverse_xy = true;
    }
    
    if (x2 < x1)
    {
        swap(&x1, &x2);
        swap(&y1, &y2);
    }
    
    int gradient = dy * 256 / dx;
    
    // handle first endpoint
    int xend = round(x1);
    int yend = y1 + gradient * (xend - x1) / 256;
    int xgap = rfpart(x1 + 128);
    int xpxl1 = xend;  // this will be used in the main loop
    int ypxl1 = ipart(yend);
    plot(xpxl1, ypxl1, rfpart(yend) * xgap / 256);
    plot(xpxl1, ypxl1 + 256, fpart(yend) * xgap / 256);
    int intery = yend + gradient; // first y-intersection for the main loop
    
    // handle second endpoint
    xend = round(x2);
    yend = y2 + gradient * (xend - x2) / 256;
    xgap = fpart(x2 + 128);
    int xpxl2 = xend;  // this will be used in the main loop
    int ypxl2 = ipart(yend);
    plot(xpxl2, ypxl2, rfpart(yend) * xgap / 256);
    plot(xpxl2, ypxl2 + 256, fpart(yend) * xgap / 256);
    
    // main loop
    for (int x = xpxl1 + 1; x <= xpxl2 - 1; x += 256)
    {
        plot(x, ipart(intery), rfpart(intery));
        plot(x, ipart(intery) + 256, fpart(intery));
        intery = intery + gradient;
    }
}

/* Non-antialiased line drawing */

void drawline(int x1, int y1, int x2, int y2, int color, int dots)
{
    // Algorithm from here: http://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm#Simplification
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    
    int err = dx - dy;
    int count = 0;
    for(;; count++)
    {
        if (!dots || (count >> (dots - 1)) & 1)
        {
            __Point_SCR(x1, y1);
            __LCD_SetPixl(color);
        }
        
        if (x1 == x2 && y1 == y2) break;
        
        int e2 = 2 * err;
        if (e2 > -dy)
        {
            err -= dy;
            x1 += sx;
        }
        else if (e2 < dx)
        {
            err += dx;
            y1 += sy;
        }
    }
}

/* Rectangle filling */

void fill_rectangle(int x, int y, int w, int h, int color)
{
    
    while (w--)
    {
        __Point_SCR(x++, y);
        __LCD_Fill((uint16_t*)&color, h);
    }
    
    __LCD_DMA_Ready();
}

void draw_rectangle(int x, int y, int w, int h, int color, int dots)
{
    drawline(x,     y,     x + w, y,     color, dots);
    drawline(x + w, y,     x + w, y + h, color, dots);
    drawline(x + w, y + h, x,     y + h, color, dots);
    drawline(x    , y + h, x,     y,     color, dots);
}

/* Small bitmaps */

// Draw a small monochrome bitmap image to screen
// The bitmap is defined as a constant array, where each entry is a
// 32 pixels wide row
void draw_bitmap(const uint32_t *bitmap, int x, int y, int color, int bitmap_size, bool center)
{
    // Find out the width of the bitmap and align it properly
    uint32_t bits = 0;
    for (int i = 0; i < bitmap_size; i++)
        bits |= bitmap[i];
    
    int width;
    
    if (center)
        width = 32 - __builtin_clz(bits) / 2;
    else
        width = 32 - __builtin_clz(bits);
    
    uint32_t mask = 1;
    for (int i = width - 1; i >= 0; i--, mask <<= 1)
    {
        for (int j = 0; j < bitmap_size; j++)
        {
            if (bitmap[j] & mask)
            {
                __Point_SCR(x + i, y + bitmap_size - j);
                __LCD_SetPixl(color);
            }
        }
    }
}