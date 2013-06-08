
#ifndef __ALTERBIOS_H__
#define __ALTERBIOS_H__

extern uint32_t alterbios_version_tag;

/* Verify that the installed AlterBIOS version is compatible with this
 * API version. Return values:
 *      Success:         positive (number of exported functions)
 *      Not installed:   -1
 *      Too old version: -2
 */
static int alterbios_check()
{
    if ((alterbios_version_tag & 0xFFFF0000) != 0x0A170000)
        return -1;
    if ((alterbios_version_tag & 0x0000FFFF) < 36)
        return -2;
    return (alterbios_version_tag & 0x0000FFFF);
}

/* Initialize AlterBIOS (basically mounts the FAT filesystem) */
extern void alterbios_init();

#endif

