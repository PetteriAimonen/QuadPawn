#include "amx_debug.h"
#include <string.h>
#include <stdio.h>

// Reads a zero-terminated string from file.
// If it is longer than dest_size, discards the rest.
static bool read_string(FIL *file, char *dest, int dest_size)
{
    unsigned bytes;
    dest_size--; // Space for terminator
    
    char byte;
    do
    {
        f_read(file, &byte, 1, &bytes);
        
        if (bytes != 1)
            return false;
        
        if (dest_size > 0)
        {
            *dest++ = byte;
            dest_size--;
        }
    } while (byte != 0);
    
    if (dest_size == 0)
    {
        *dest = 0;
    }
    
    return true;
}

// Read one entry from the file table
static bool read_filetbl(FIL *file, unsigned *address, char *name, int name_size)
{
    unsigned bytes;
    f_read(file, address, 4, &bytes);
    if (bytes != 4) return false;
    
    return read_string(file, name, name_size);
}

static bool read_symboltbl(FIL *file, AMX_DBG_SYMBOL *symbol)
{
    unsigned bytes;
    f_read(file, symbol, 18, &bytes);
    if (bytes != 18) return false;
    
    if (!read_string(file, symbol->name, sizeof(symbol->name)))
        return false;
    
    for (int i = 0; i < symbol->dim; i++)
    {
        f_read(file, symbol->dims + i, sizeof(AMX_DBG_SYMDIM), &bytes);
    }
    
    return true;
}

bool amxdbg_load(FIL* file, AMX_DEBUG_INFO *dbg)
{
    unsigned debug_start;
    unsigned bytes;
    
    dbg->file = file;
    
    // Read the file size from the main AMX header and use that to seek
    // to the debug data.
    f_lseek(file, 0);
    f_read(file, &debug_start, sizeof(unsigned), &bytes);
    f_lseek(file, debug_start);
    f_read(file, &dbg->header, sizeof(AMX_DBG_HDR), &bytes);
    
    if (bytes != sizeof(AMX_DBG_HDR) || dbg->header.magic != AMX_DBG_MAGIC)
        return false;
    
    // Fill in the table offsets
    dbg->filetbl_offset = f_tell(file);
    
    // Seek to end of the files table
    for (int i = 0; i < dbg->header.files; i++)
    {
        unsigned dummy;
        if (!read_filetbl(file, &dummy, NULL, 0))
            return false;
    }
    
    dbg->linetbl_offset = f_tell(file);
    dbg->symboltbl_offset = dbg->linetbl_offset + dbg->header.lines * sizeof(AMX_DBG_LINE);
    
    return true;
}

bool amxdbg_find_location(AMX_DEBUG_INFO *dbg, unsigned instruction_pointer,
                          char *location, unsigned location_size)
{
    unsigned bytes, address;
    
    // Find filename
    f_lseek(dbg->file, dbg->filetbl_offset);
    for (int i = 0; i < dbg->header.files; i++)
    {
        char tmp[100];
        if (!read_filetbl(dbg->file, &address, tmp, sizeof(tmp)))
            return false;
        
        if (address > instruction_pointer)
        {
            // Previous entry was the correct one
            break;
        }
        else
        {
            // Get the last part of the file name (basename)
            char *p = strrchr(tmp, '/');
            if (!p) p = tmp;
            
            location[0] = 0;
            strncat(location, p, location_size - 1);
        }
    }
    
    // Find line number
    int line = -1;
    f_lseek(dbg->file, dbg->linetbl_offset);
    for (int i = 0; i < dbg->header.lines; i++)
    {
        AMX_DBG_LINE tmp;
        f_read(dbg->file, &tmp, sizeof(tmp), &bytes);
        if (bytes != sizeof(tmp)) return false;
        
        if (tmp.address > instruction_pointer)
            break;
        else
            line = tmp.line + 1; // Lines are 0-indexed in debug info.
    }
    
    if (line == -1)
        return false;
    
    // Find function name
    AMX_DBG_SYMBOL symbol;
    char *funcname = "???";
    f_lseek(dbg->file, dbg->symboltbl_offset);
    for (int i = 0; i < dbg->header.symbols; i++)
    {
        if (!read_symboltbl(dbg->file, &symbol))
            return false;
        
        if (symbol.ident == iFUNCTN
            && symbol.codestart <= instruction_pointer
            && symbol.codeend > instruction_pointer)
        {
            funcname = symbol.name;
            break;
        }
    }
    
    // Format result
    int pos = strlen(location);
    snprintf(location + pos, location_size - pos,
             ":%d inside %s()", line, funcname);
    
    return true;
}

bool amxdbg_format_locals(AMX_DEBUG_INFO *dbg, AMX *amx,
                          unsigned frame, unsigned instruction,
                          char *dest, int dest_size)
{
    AMX_HEADER *hdr = (AMX_HEADER*)amx->base;
    cell *dat = (cell*)(amx->base + hdr->dat);
    unsigned frm = frame / 4;
    
    dest[0] = 0;
    
    AMX_DBG_SYMBOL symbol;
    f_lseek(dbg->file, dbg->symboltbl_offset);
    for (int i = 0; i < dbg->header.symbols && dest_size > 0; i++)
    {
        if (!read_symboltbl(dbg->file, &symbol))
            return false;
        
        if (symbol.ident == iVARIABLE
            && symbol.vclass != 0
            && symbol.codestart <= instruction
            && symbol.codeend > instruction)
        {
            cell value;
            if (symbol.vclass == 1)
                value = dat[frm + symbol.address / 4];
            else
                value = dat[symbol.address / 4];
            
            int bytes = snprintf(dest, dest_size, "%s = %d  ",
                symbol.name, (int)value);
            dest += bytes;
            dest_size -= bytes;
        }
    }
    
    return true;
}

unsigned amxdbg_get_caller(AMX *amx, unsigned *frame)
{
    AMX_HEADER *hdr = (AMX_HEADER*)amx->base;
    cell *dat = (cell*)(amx->base + hdr->dat);
    
    unsigned frm = *frame / 4;
    unsigned addr = dat[frm + 1];
    
    printf("stack from %08x: %08x %08x %08x %08x\n",
           (unsigned)*frame, (unsigned)dat[frm], (unsigned)dat[frm + 1], (unsigned)dat[frm + 2], (unsigned)dat[frm + 3]);
    
    *frame = dat[frm] - (cell)dat;
    
    return addr;
}



