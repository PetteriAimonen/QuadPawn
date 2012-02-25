/* Glue for FatFS accessing the DSO Quad internal disc.
 * This program uses FatFS instead of the built-in BIOS file system library,
 * because the built-in library does not support e.g. getting a file list.
 */

#include "diskio.h"
#include "BIOS.h"

DSTATUS disk_initialize(BYTE drv)
{
    // Supports only one drive, no initialization necessary.
    return (drv == 0) ? 0 : STA_NOINIT;
}

DSTATUS disk_status (BYTE drv)
{
    return (drv == 0) ? 0 : STA_NOINIT;
}

DRESULT disk_read(BYTE drv, BYTE *buff, DWORD sector, BYTE count)
{
    if (drv != 0 || count == 0) return RES_PARERR;
    
    while (count--)
    {
        // The hardware page size is 256 bytes
        if (__ReadDiskData(buff, sector * 512, 256) != 0)
            return RES_ERROR;
        
        if (__ReadDiskData(buff + 256, sector * 512 + 256, 256) != 0)
            return RES_ERROR;
        
        buff += 512;
    }
    
    return RES_OK;
}

DRESULT disk_write(BYTE drv, const BYTE *buff, DWORD sector, BYTE count)
{
    if (drv != 0 || count == 0) return RES_PARERR;
    
    while (count--)
    {
        if (__ProgDiskPage((u8*)buff, sector * 512) != 0)
            return RES_ERROR;
        
        if (__ProgDiskPage((u8*)buff + 256, sector * 512 + 256) != 0)
            return RES_ERROR;
        
        buff += 512;
    }
    
    return RES_OK;
}

DRESULT disk_ioctl(BYTE drv, BYTE ctrl, void *buff)
{
    if (drv != 0) return RES_PARERR;
    
    if (ctrl == CTRL_SYNC)
    {
        return RES_OK;
    }
    else if (ctrl == GET_SECTOR_COUNT)
    {
        *(DWORD*)buff = 4096;
        return RES_OK;
    }
    else if (ctrl == GET_SECTOR_SIZE || ctrl == GET_BLOCK_SIZE)
    {
        *(DWORD*)buff = 512;
        return RES_OK;
    }
    else
    {
        return RES_PARERR;
    }
}

DWORD get_fattime (void)
{
        /* Pack date and time into a DWORD variable */
        return    ((DWORD)(2000 - 1980) << 25)
                        | ((DWORD)1 << 21)
                        | ((DWORD)1 << 16)
                        | ((DWORD)13 << 11)
                        | ((DWORD)37 << 5)
                        | ((DWORD)0 >> 1);
}
