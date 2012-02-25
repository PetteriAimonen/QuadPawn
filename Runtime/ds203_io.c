#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "ds203_io.h"
#include "BIOS.h"
#include <stdio.h>
#include "mathutils.h"
#include "drawing.h"
#include "ff.h"

/* ---- LCD drawing ---- */

static char buffer[55];

// Format the message to statically allocated buffer and then call BIOS to
// print it on screen.
int lcd_printf(u16 x0, u16 y0, u16 color, u8 mode, const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    int rv = vsnprintf(buffer, sizeof(buffer), fmt, va);
    va_end(va);
    
    __Display_Str(x0, y0, color, mode, (u8*)buffer);
    return rv;
}

// Find a filename that is not in use. Format is a printf format string.
// char *select_filename(const char *format)
// {
//     static char filename[13];
//     for (int i = 0; i <= 999; i++)
//     {
//         snprintf(filename, sizeof(filename), format, i);
//         if (!_fexists(filename)) break;
//     }
//     return filename;
// }


/* ----------------- Bitmap writing ------------ */

// 16-color palette for bitmaps. Could be adjusted per-application to
// get better looking images.
static uint16_t palette[16] = {
    RGB565RGB(0,0,0), RGB565RGB(255,255,255), RGB565RGB(128,128,128),
    RGB565RGB(255,0,0), RGB565RGB(0,255,0), RGB565RGB(0,0,255),
    RGB565RGB(255,255,0), RGB565RGB(0,255,255), RGB565RGB(255,0,255),
    RGB565RGB(128,0,0), RGB565RGB(0,128,0), RGB565RGB(0,0,128),
    
    // Following 4 places are used for application-specific colors
    RGB565RGB(63, 63, 63)
};

// Find the closest color in palette, using Manhattan distance.
// Comparing in HSV space would be more accurate but slower.
int quantize(uint16_t color)
{
    int min_delta = 999999;
    int closest = 0;
    for (int i = 0; i < 16; i++)
    {
        int delta = (
            abs(RGB565_R(color) - RGB565_R(palette[i])) +
            abs(RGB565_G(color) - RGB565_G(palette[i])) +
            abs(RGB565_B(color) - RGB565_B(palette[i]))
        );
        if (delta < min_delta)
        {
            min_delta = delta;
            closest = i;
        }
    }
    
    return closest;
}

// We know the LCD size in advance, so the file header can be written out
// by hand.
// The main header is 14 bytes, bitmap info header is 40 bytes and the palette
// is 64 bytes. Therefore bitmap data starts at offset 118.
const uint8_t BMP_HEADER[54] = {
    0x42, 0x4D, // BMP magic
    0xF6, 0xBB, 0x00, 0x00, // File size, 400 * 240 / 2 + 118 = 48118 bytes
    0x00, 0x00, 0x00, 0x00, // Creator
    0x76, 0x00, 0x00, 0x00, // Offset to bitmap data = 118 bytes 
    0x28, 0x00, 0x00, 0x00, // Header size = 40 bytes
    0x90, 0x01, 0x00, 0x00, // Bitmap width = 400 pixels
    0xF0, 0x00, 0x00, 0x00, // Bitmap height = 240 pixels
    0x01, 0x00,             // 1 color plane
    0x04, 0x00,             // 4 bits per pixel
    0x00, 0x00, 0x00, 0x00, // Uncompressed
    0x80, 0xBB, 0x00, 0x00, // Bitmap data size, 400 * 240 / 2 = 48000 bytes
    0x70, 0x17, 0x00, 0x00, // Horizontal pixel per meter = 6000
    0x70, 0x17, 0x00, 0x00, // Vertical pixel per meter
    0x10, 0x00, 0x00, 0x00, // Palette colors = 16
    0x00, 0x00, 0x00, 0x00  // Important colors
};

bool write_bitmap(const char *filename)
{
    FIL file;
    
    if (f_open(&file, filename, FA_WRITE | FA_CREATE_ALWAYS) != FR_OK)
        return false;
    
    // Write header
    unsigned bytes;
    f_write(&file, BMP_HEADER, sizeof(BMP_HEADER), &bytes);
    if (bytes != sizeof(BMP_HEADER))
        return false;
    
    // Write palette
    for (int i = 0; i < 16; i++)
    {
        f_putc(RGB565_B(palette[i]), &file);
        f_putc(RGB565_G(palette[i]), &file);
        f_putc(RGB565_R(palette[i]), &file);
        f_putc(0, &file);
    }
    
    // Write bitmap data
    for (int y = 0; y < 240; y++)
    {
        for (int x = 0; x < 400; x += 2)
        {
            __Point_SCR(x, y);
            int colorH = quantize(__LCD_GetPixl());
            __Point_SCR(x + 1, y);
            int colorL = quantize(__LCD_GetPixl());
            
            f_putc((colorH << 4) | colorL, &file);
        }
    }
    
    return f_close(&file) == FR_OK;
}



