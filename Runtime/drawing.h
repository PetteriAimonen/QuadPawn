/* Functions for drawing on the DS203 lcd */

#pragma once
#include <stdbool.h>
#include "fix16.h"

// Macros for converting from normal (r,g,b) 0-255 values to 16-bit RGB565
// and back again. Use e.g. RGB(255,0,0) for bright red.
#define RGB565RGB(r, g, b) (((r)>>3)|(((g)>>2)<<5)|(((b)>>3)<<11))
#define RGB(r,g,b) RGB565RGB(r,g,b)
#define RGB565_R(color) (int)((color & 0x001F) << 3)
#define RGB565_G(color) (int)((color & 0x07E0) >> 3)
#define RGB565_B(color) (int)((color & 0xF800) >> 8)

// You can pass -1 as bg to have transparent background.
void draw_text(const char *text, int x, int y, int fg, int bg, bool center);

// Returns number of lines in the text
int draw_flowtext(const char *text, int x, int y, int w, int h, int fg, int bg, bool center);

void drawline_aa(fix16_t fx1, fix16_t fy1, fix16_t fx2, fix16_t fy2, int color);

void drawline(int x1, int y1, int x2, int y2, int color, int dots);

void fill_rectangle(int x, int y, int w, int h, int color);

void draw_rectangle(int x, int y, int w, int h, int color, int dots);

void draw_bitmap(const uint32_t *bitmap, int x, int y, int color, int bitmap_size, bool center);
