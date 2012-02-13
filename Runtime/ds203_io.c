#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "ds203_io.h"
#include "BIOS.h"
#include "Interrupt.h"
#include <stdio.h>
#include "mathutils.h"

/* ---- LCD drawing ---- */

static struct {
    uint32_t PC;
    uint32_t LR;
    uint32_t SP;
    uint32_t R7;
    char BUILDID[32];
} CRASHDATA;

static const char BUILDID[32] = __DATE__ " " __TIME__;

void crash_with_message(const char *message, void *caller)
{
    asm("mov %0, pc" : "=r"(CRASHDATA.PC) : :);
    asm("mov %0, lr" : "=r"(CRASHDATA.LR) : :);
    asm("mov %0, sp" : "=r"(CRASHDATA.SP) : :);
    asm("mov %0, r7" : "=r"(CRASHDATA.R7) : :);
    memcpy(CRASHDATA.BUILDID, BUILDID, sizeof(BUILDID));
    
    __Clear_Screen(0b0000000000011111);
    __Set(BEEP_VOLUME, 0);
    
    int y = 220;
    lcd_printf(10, y, 0xFFFF, 0, "      %s      ", message);
    y -= 14;
    
    lcd_printf(10, y, 0xFFFF, 0, "Caller: %08lx",
        (uint32_t)caller
    );
    y -= 14;
    
    uint32_t* sp = __builtin_frame_address(0);
    lcd_printf(10, y, 0xFFFF, 0, "Raw stack (from %08lx):", (uint32_t)sp);
    y -= 14;
    
    while (y > 30)
    {
        lcd_printf(10, y, 0xFFFF, 0, "  %08lx %08lx %08lx %08lx",
                   sp[0], sp[1], sp[2], sp[3]);
        y -= 14;
        sp += 4;
    }
    
    debugf("Dumping memory...");
    _fopen_wr("memory.dmp");
    for (char *p = (char*)0x20000000; p < (char*)0x2000C000; p++)
        _fputc(*p);
    if (_fclose())
        debugf("Memory dumped to memory.dmp");
    else
        debugf("Memory dump failed (disc full?).");
    
    while(1);
}


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

// Show a single-line menu of selections and loop until user makes a selection
int show_menu(u16 x0, u16 y0, u16 color, const char *prompt, ...)
{
    va_list va;
    int current_selection = 0;
    while(1)
    {
        clearline(y0);
        __Display_Str(x0, y0, color, 0, (u8*)prompt);
        
        int item_count = 0;
        int x = x0 + strlen(prompt) * FONT_WIDTH + 16;
        const char *choice;
        va_start(va, prompt);
        while ((choice = va_arg(va, const char *)) != NULL)
        {
            int mode = (item_count == current_selection) ? 1 : 0;
            __Display_Str(x, y0, color, mode, (u8*)choice);
            x += strlen(choice) * FONT_WIDTH + 16;
            item_count++;
        }
        va_end(va);

        DelayMs(100);
        
        // Wait until key is pressed
        int key;
        do {
            key = __Get(KEY_STATUS) ^ ALL_KEYS;
        } while (!(key & (K_ITEM_D_STATUS | K_ITEM_S_STATUS | K_ITEM_I_STATUS)));
        
        // Wait for key to release
        while (__Get(KEY_STATUS) ^ ALL_KEYS);
        DelayMs(100);
        
        if ((key & K_ITEM_D_STATUS) && current_selection > 0)
            current_selection--;
        else if ((key & K_ITEM_I_STATUS) && current_selection < item_count - 1)
            current_selection++;
        else if (key & K_ITEM_S_STATUS)
            return current_selection;
    }
}

// Alpha-blend two colors together, helper for drawline.
static u16 adjust_alpha(u16 newcolor, u16 oldcolor, int alpha)
{
    alpha = 256 - (256 - alpha) * 2 / 3; // To make it look better on cheap TFT :)
    int invalpha = 256 - alpha;
    
    int r = RGB565_R(newcolor);
    int g = RGB565_G(newcolor);
    int b = RGB565_B(newcolor);
    int oldr = RGB565_R(oldcolor);
    int oldg = RGB565_G(oldcolor);
    int oldb = RGB565_B(oldcolor);
    
    r = (r * alpha + oldr * invalpha) / 256;
    g = (g * alpha + oldg * invalpha) / 256;
    b = (b * alpha + oldb * invalpha) / 256;
    return RGB565RGB(r, g, b);
}

// Draws antialiased lines
// Xiaolin Wu's algorithm, using x/256 fixed point values
void drawline(float fx1, float fy1, float fx2, float fy2, u16 color)
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
        __LCD_SetPixl(adjust_alpha(color, oldcolor, c));
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
    
    int x1 = (int)(fx1 * 256);
    int x2 = (int)(fx2 * 256);
    int y1 = (int)(fy1 * 256);
    int y2 = (int)(fy2 * 256);

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

// Draw 1-1 pattern dotted line horizontally from (x1,y) to (x2,y)
void horizdots(int x1, int x2, int y, u16 color)
{
    for (int x = x1; x <= x2; x += 2)
    {
         __Point_SCR(x, y);
        __LCD_SetPixl(color);
    }
}

// Draw 1-1 pattern dotted line vertically from (x,y1) to (x,y2)
void vertdots(int x, int y1, int y2, u16 color)
{
    for (int y = y1; y <= y2; y += 2)
    {
         __Point_SCR(x, y);
        __LCD_SetPixl(color);
    }
}

/* ---- File IO ---- */

// To simplify matters, especially related to memory allocation, this code
// uses statically allocated sector buffer. Therefore we can only have one
// open file at a time.

static u16 pCluster[3];
static u32 pDirAddr[1];
// Err.. something somewhere is writing past SecBuff, and I'm quite sure it is
// not my code. Allocating a bit more than 512 bytes for that purpose.
static u8 SecBuff[600];
static uint32_t file_length;
static uint32_t secbuff_pos;
static uint32_t sector_count;
static bool file_ok = false;

// The BIOS doesn't want the dot to be included in the name.
// I think it is more natural to type "foo.csv" than "foo     csv", therefore
// this function makes the conversion.
// It returns a pointer to a static array so it is not thread-safe, but we
// can only have one open file at a time anyway.
static u8 *fix_filename(const char *filename)
{
    static u8 result[11];
    const char *p = filename;
    int i = 0;
    while (*p)
    {
        if (*p == '.')
        {
            p++;
            while (i < 8) result[i++] = ' ';
        }
        
        result[i++] = *p++;
    }
    while (i < 11) result[i++] = ' ';
    return result;
}

// Try to open a file and return true if it succeeds
bool _fexists(const char *filename)
{
    // There is no need to close the file, as OpenFileRd doesn't write anything
    // to disc.
    return (__OpenFileRd(SecBuff, fix_filename(filename), pCluster, pDirAddr) == 0);
}

// Open a file for writing, creating it if it doesn't already exist.
bool _fopen_wr(const char *filename)
{
    int status = __OpenFileWr(SecBuff, fix_filename(filename), pCluster, pDirAddr);    
    file_ok = (status == 0);
    file_length = 0;
    secbuff_pos = 0;
    sector_count = 0;
    return file_ok;
}

// Write one character to SecBuff. If the sector is complete, write it to
// disc and start the next one.
void _fputc(char c)
{
    if (!file_ok) return;
    
    SecBuff[secbuff_pos++] = c;
    file_length++;
    
    if (secbuff_pos >= 512)
    {
        secbuff_pos = 0;
        sector_count++;
        if (__ProgFileSec(SecBuff, pCluster) != 0)
        {
            file_ok = false; // Write error
        }
    }
}

// Hackish
typedef void (*putcf) (void *, char);
void tfp_format(void *putp, putcf putf, const char *fmt, va_list va);

// Wrapper of _fputc for tinyprintf.
static void file_putp(void *p, char c)
{
    _fputc(c);
}

// Close the currently open file. Checks the error status and returns false
// if any error has occurred since opening the file.
bool _fclose()
{
    if (!file_ok) return false;
    
    // The bios only supports file sizes a multiple of 512
    // Therefore we pad the files with spaces.
    while (secbuff_pos != 0) _fputc(' ');
    
    file_ok = false;
    int status = __CloseFile(SecBuff, sector_count * 512, pCluster, pDirAddr);
    return status == 0;
}

// Printf directly to file
int _fprintf(const char *fmt, ...)
{
    uint32_t first_length = file_length;
    va_list va;
    va_start(va, fmt);
    tfp_format(NULL, file_putp, fmt, va);
    va_end(va);
    
    return file_length - first_length;
}

// Find a filename that is not in use. Format is a printf format string.
char *select_filename(const char *format)
{
    static char filename[13];
    for (int i = 0; i <= 999; i++)
    {
        snprintf(filename, sizeof(filename), format, i);
        if (!_fexists(filename)) break;
    }
    return filename;
}


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
    _fopen_wr(filename);
    
    // Write header
    for (int i = 0; i < 54; i++)
    {
        _fputc(BMP_HEADER[i]);
    }
    
    // Write palette
    for (int i = 0; i < 16; i++)
    {
        _fputc(RGB565_B(palette[i]));
        _fputc(RGB565_G(palette[i]));
        _fputc(RGB565_R(palette[i]));
        _fputc(0);
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
            
            _fputc((colorH << 4) | colorL);
        }
    }
    
    return _fclose();
}


/* ---- Misc ---- */

// Waits for the specified number of milliseconds
void DelayMs(u16 ms)
{
    // The count is decremented in an interrupt
    Delay_Cnt = ms;
    while (Delay_Cnt > 0);
}



