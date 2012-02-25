// Useful IO routines for developing on DS203
// Petteri Aimonen 2011

#pragma once

#include <stm32f10x.h>
#include <stdbool.h>
#include <stdarg.h>
#include <fix16.h>

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

/* ------------- File IO ------------- */
// Select a free filename using a printf-style template.
// Goes through parameters 0 to 999 until a non-existent filename is found.
// Returns pointer to a static array, so next call will overwrite the value.
// E.g. select_filename("FILE%03d.CSV") might give "FILE001.CSV".
// char *select_filename(const char *format);

// Write current LCD contents as a bitmap into file.
// (no other file can be open simultaneously)
// You can add the colours you use to the list in ds203_io.c to make
// the result look better.
bool write_bitmap(const char *filename);
