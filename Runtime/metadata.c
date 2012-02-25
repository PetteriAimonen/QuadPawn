/* Functions for accessing the metadata stored in .amx files.
 * In order to not hang on invalid files, this uses manual parsing instead
 * of the real virtual machine.
 */

#include "amx.h"
#include "ff.h"
#include "metadata.h"
#include <string.h>

// Returns code offset or -1.
static int find_public_function(AMX_HEADER *hdr, FIL *file, const char *name)
{
    if (hdr->defsize < sizeof(AMX_FUNCSTUB))
        return -1;
    
    int count = (hdr->natives - hdr->publics) / hdr->defsize;
    
    for (int i = 0; i < count; i++)
    {
        f_lseek(file, hdr->publics + hdr->defsize * i);
        
        AMX_FUNCSTUB func;
        unsigned bytes;
        f_read(file, &func, sizeof(AMX_FUNCSTUB), &bytes);
        if (bytes != sizeof(AMX_FUNCSTUB))
            return -1;
        
        char buffer[20];
        f_lseek(file, func.nameofs);
        f_read(file, buffer, sizeof(buffer), &bytes);
        
        if (strncmp(buffer, name, 20) == 0)
        {
            return func.address;
        }
    }
    
    return -1;
}

#define OPCODE_CONST_PRI 9
#define OPCODE_MOVS 64

// Finds the first constant data array reference in the function
static bool get_data_array(AMX_HEADER *hdr, FIL *file, const char *function,
                           int *result_dataptr, int *result_size)
{
    int address = find_public_function(hdr, file, function);
    if (address < 0)
        return false;
    
    f_lseek(file, hdr->cod + address);
    unsigned bytes;
    uint32_t opcode;
    int maxcount = 20;
    
    do {
        f_read(file, &opcode, 4, &bytes);
        if (!maxcount--) return false;
    } while (opcode != OPCODE_CONST_PRI);
    
    f_read(file, result_dataptr, 4, &bytes);
    if (bytes != 4)
        return false;
    
    do {
        f_read(file, &opcode, 4, &bytes);
        if (!maxcount--) return false;
    } while (opcode != OPCODE_MOVS && maxcount-- > 0);
    
    f_read(file, result_size, 4, &bytes);
    if (bytes != 4)
        return false;
    
    return true;
}

bool get_program_metadata(const char *filename, uint32_t icon[32], int *icon_size, char *name, int name_size)
{
    AMX_HEADER hdr;
    FIL file;
    unsigned bytes;
    
    FRESULT status = f_open(&file, filename, FA_READ);
    if (status != FR_OK)
        return false;
    
    f_read(&file, &hdr, sizeof(hdr), &bytes);
    if (bytes != sizeof(hdr) || hdr.magic != AMX_MAGIC)
    {
        f_close(&file);
        return false;
    }
    
    int bytecount;
    int icon_dataptr;
    if (get_data_array(&hdr, &file, "get_program_icon", &icon_dataptr, &bytecount))
    {
        *icon_size = bytecount / 4;
        if (*icon_size > 32) *icon_size = 32;
        
        f_lseek(&file, hdr.dat + icon_dataptr);
        f_read(&file, icon, *icon_size * 4, &bytes);
    }
    
    int name_dataptr;
    if (get_data_array(&hdr, &file, "get_program_name", &name_dataptr, &bytecount))
    {
        f_lseek(&file, hdr.dat + name_dataptr);
        f_read(&file, name, name_size, &bytes);
        
        for (int i = 0; i + 4 <= name_size; i += 4)
        {
            uint32_t *p = (uint32_t*)(name + i);
            *p = __builtin_bswap32(*p);
        }
        
        name[name_size - 1] = 0; // Just to make sure that it has terminator somewhere
    }
    
    f_close(&file);
    return true;
}