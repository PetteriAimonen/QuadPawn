/* This is a bunch of functions to print traceback information when a
 * Pawn program crashes. It differs from the stock amxdbg.c by accessing
 * the debug information from the file instead of loading it all to RAM.
 */

#pragma once
#include <stdbool.h>
#include "ff.h"
#include "amxdbg.h" // For definition of AMX_DBG_HDR

typedef struct {
    AMX_DBG_HDR header;
    FIL *file;
    const AMX *amx;
    
    // Offsets to the tables, relative to file start
    unsigned filetbl_offset;
    unsigned linetbl_offset;
    unsigned symboltbl_offset;
} AMX_DEBUG_INFO;

// Load the debug header and find table locations in the file
// Note: the file pointer is stored in dbg, so dbg will be invalid
// once the file is closed.
bool amxdbg_load(FIL* file, const AMX* amx, AMX_DEBUG_INFO *dbg);

// Find the file and line that correspond to given instruction
bool amxdbg_find_location(AMX_DEBUG_INFO *dbg, unsigned instruction_pointer,
                          char *location, unsigned location_size);

// Format locals and their values to a string
bool amxdbg_format_locals(AMX_DEBUG_INFO *dbg, AMX *amx,
                          unsigned frame, unsigned instruction_pointer,
                          char *dest, int dest_size);

// Get the previous caller from the call stack and the frame pointer of it
// Returns the instruction pointer to the CALL instruction
unsigned amxdbg_get_caller(AMX *amx, unsigned *frame);
