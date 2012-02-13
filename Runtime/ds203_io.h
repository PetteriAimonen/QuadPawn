// Useful IO routines for developing on DS203
// Petteri Aimonen 2011

#pragma once

#include <stm32f10x.h>
#include <stdbool.h>
#include <stdarg.h>

/* ----------- Text printing ------------ */

// Print stack trace and display a message
void crash_with_message(const char *message, void *caller);

// Print to lcd with specified position & color. Mode can be 0 for normal text
// or 1 for inverted colors (black text on hilighted background).
int lcd_printf(u16 x0, u16 y0, u16 color, u8 mode, const char *fmt, ...)
__attribute__ ((format (printf, 5, 6)));;

// Shorthand function for printing debug messages. Always writes on the bottom
// row of the screen.
#define debugf(...) lcd_printf(0, 0, 0xFFFF, 0, __VA_ARGS__)

// Helper function for clearing a text row before printing.
#define clearline(y0) lcd_printf(0, y0, 0, 0, "                                                                       ")

// Show single-line selection menu with given choices.
// Returns index 0...n of the selected item.
// The last item given must be NULL to terminate the list.
// E.g. int choice = show_menu(0, 0, 0xFFFF, "Select:", "Duck", "Cat", NULL);
int show_menu(u16 x0, u16 y0, u16 color, const char *prompt, ...);

/* ------------- Graphics ---------------- */

// Macros for converting from normal (r,g,b) 0-255 values to 16-bit RGB565
// and back again. Use e.g. RGB565RGB(255,0,0) for bright red.
#define RGB565RGB(r, g, b) (((r)>>3)|(((g)>>2)<<5)|(((b)>>3)<<11))
#define RGB565_R(color) (int)((color & 0x001F) << 3)
#define RGB565_G(color) (int)((color & 0x07E0) >> 3)
#define RGB565_B(color) (int)((color & 0xF800) >> 8)

// Draw antialiased line.
void drawline(float fx1, float fy1, float fx2, float fy2, u16 color);

// Draw horizontal or vertical dotted line.
void horizdots(int x1, int x2, int y, u16 color);
void vertdots(int x, int y1, int y2, u16 color);

/* ------------- File IO ------------- */

// These functions allow writing to files.
// Current limitations (only in ds203_io.c, BIOS supports these):
// 1) No file reading
// 2) Only one open file at a time
//
// To simplify error handling, all errors are queued and further actions are
// ignored. You can simply write everything without checks and then check the
// status of _fclose() to see if there were any errors. True means success.
//
// Example usage:
// _fopen_wr("myfile.csv");
// _fprintf("Hello there!\r\n");
// if (!_fclose()) debugf("Writing failed!");

bool _fexists(const char *filename);
bool _fopen_wr(const char *filename);
void _fputc(char c);
int _fprintf(const char *fmt, ...);
bool _fclose();

// Select a free filename using a printf-style template.
// Goes through parameters 0 to 999 until a non-existent filename is found.
// Returns pointer to a static array, so next call will overwrite the value.
// E.g. select_filename("FILE%03d.CSV") might give "FILE001.CSV".
char *select_filename(const char *format);

// Write current LCD contents as a bitmap into file.
// (no other file can be open simultaneously)
// You can add the colours you use to the list in ds203_io.c to make
// the result look better.
bool write_bitmap(const char *filename);

/* ----------- Miscellaneous ----------- */

// Delay specified number of milliseconds.
void DelayMs(u16 ms);

