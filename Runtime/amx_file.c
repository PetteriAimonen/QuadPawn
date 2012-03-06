/* Pawn interface to fatfs */

#include "ff.h"
#include "amx.h"
#include <stdio.h>
#include <stdbool.h>

#define FILE_COUNT 4
static FIL files[FILE_COUNT];

static cell AMX_NATIVE_CALL amx_f_open(AMX *amx, const cell *params)
{
    FIL *file = 0;
    for (int i = 0; i < FILE_COUNT; i++)
    {
        if (files[i].fs == 0)
        {
            file = &files[i];
            break;
        }
    }
    
    if (!file)
        return 0; // Out of file descriptors
    
    char *fname;
    amx_StrParam(amx, params[1], fname);
    FRESULT status = f_open(file, fname, params[2]);
    
    if (status != FR_OK)
    {
        printf("f_open failed with %d\n", status);
        file->fs = 0;
        return 0;
    }
    else
    {
        return (cell)file;
    }
}

static cell AMX_NATIVE_CALL amx_f_close(AMX *amx, const cell *params)
{
    FIL* file = (FIL*)params[1];
    if (!file) return false;
    
    FRESULT status = f_close(file);
    file->fs = 0; // Make sure that the file entry is marked as free
    
    return (status == FR_OK);
}

static cell AMX_NATIVE_CALL amx_f_read(AMX *amx, const cell *params)
{
    FIL* file = (FIL*)params[1];
    if (!file) return 0;
    
    unsigned bytes;
    FRESULT status = f_read(file, (char*)params[2], params[3] * 4, &bytes);
    
    // Swap the bytes in each cell to convert into Pawn packed string
    cell* p = (cell*)params[2];
    for (int i = 0; i < params[3]; i++, p++)
    {
        *p = __builtin_bswap32(*p);
    }
    
    return bytes;
}

static cell AMX_NATIVE_CALL amx_f_write(AMX *amx, const cell *params)
{
    FIL* file = (FIL*)params[1];
    if (!file) return false;
    
    cell* src = (cell*)params[2];
    int count = params[3];
    
    if (count < 0)
    {
        char *text;
        amx_StrParam(amx, src, text);
        unsigned bytes;
        unsigned size = strlen(text);
        return (f_write(file, text, size, &bytes) == FR_OK && bytes != size);
    }
    else
    {
        while (count > 0)
        {
            uint32_t tmp = __builtin_bswap32(*src++);
            unsigned bytes;
            unsigned size = 4;
            if (count < size) size = count;
            
            if (f_write(file, &tmp, size, &bytes) != FR_OK || bytes != size)
                return false;
            
            count -= 4;
        }
        return true;
    }
}

static cell AMX_NATIVE_CALL amx_f_lseek(AMX *amx, const cell *params)
{
    FIL* file = (FIL*)params[1];
    if (!file) return false;

    return f_lseek(file, params[2]) == FR_OK;
}

static cell AMX_NATIVE_CALL amx_f_tell(AMX *amx, const cell *params)
{
    FIL* file = (FIL*)params[1];
    if (!file) return 0;

    return f_tell(file);
}

static cell AMX_NATIVE_CALL amx_f_size(AMX *amx, const cell *params)
{
    FIL* file = (FIL*)params[1];
    if (!file) return 0;

    return f_size(file);
}

static cell AMX_NATIVE_CALL amx_f_unlink(AMX *amx, const cell *params)
{
    char *text;
    amx_StrParam(amx, params[1], text);
    return f_unlink(text) == FR_OK;
}

static cell AMX_NATIVE_CALL amx_f_getfree(AMX *amx, const cell *params)
{
    DWORD clusters;
    FATFS *fs;
    if (f_getfree("", &clusters, &fs) != FR_OK) return 0;
    
    return clusters * fs->csize;
}

static DIR root_dir;
static cell AMX_NATIVE_CALL amx_f_opendir(AMX *amx, const cell *params)
{
    return f_opendir(&root_dir, "");
}

static cell AMX_NATIVE_CALL amx_f_readdir(AMX *amx, const cell *params)
{
    FILINFO info;
    if (f_readdir(&root_dir, &info) != FR_OK || info.fname[0] == 0)
        return false;
    
    amx_SetString((cell*)params[1], info.fname, true, false, 13);
    *(cell*)params[2] = info.fsize;
    
    return true;
}

int amxinit_file(AMX *amx)
{
    static const AMX_NATIVE_INFO funcs[] = {
        {"f_open", amx_f_open},
        {"f_close", amx_f_close},
        {"f_read", amx_f_read},
        {"f_write", amx_f_write},
        {"f_lseek", amx_f_lseek},
        {"f_tell", amx_f_tell},
        {"f_size", amx_f_size},
        {"f_getfree", amx_f_getfree},
        {"f_opendir", amx_f_opendir},
        {"f_readdir", amx_f_readdir},
        {0, 0}
    };
    
    // Clear the file table
    for (int i = 0; i < FILE_COUNT; i++)
    {
        files[i].fs = 0;
    }
    
    return amx_Register(amx, funcs, -1);
}