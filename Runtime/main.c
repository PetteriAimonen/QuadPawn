#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "BIOS.h"
#include "stm32f10x.h"
#include "ds203_io.h"
#include "Interrupt.h"
#include "ff.h"
#include "amx.h"
#include "menubar.h"
#include "buttons.h"
#include "file_selector.h"
#include "amxaux.h"
#include "msgbox.h"
#include "amx_debug.h"

// For some reason, the headers don't have this register
#define FSMC_BTR1   (*((vu32 *)(0xA0000000+0x04)))
#define FSMC_BTR2   (*((vu32 *)(0xA0000008+0x04)))

FATFS fatfs;
AMX amx;

// Data block allocated for the virtual machine
char vm_data[30000];

int amxinit_display(AMX *amx);
int amx_CoreInit(AMX *amx);
int amxinit_string(AMX *amx);
int amxinit_fixed(AMX *amx);
int amxinit_wavein(AMX *amx);
int amxinit_waveout(AMX *amx);
int amxinit_menu(AMX *amx);
int amx_menu_doevents(AMX *amx);

// Copied from amx.c
#define NUMENTRIES(hdr,field,nextfield) \
                        (unsigned)(((hdr)->nextfield - (hdr)->field) / (hdr)->defsize)
#define GETENTRY(hdr,table,index) \
                        (AMX_FUNCSTUB *)((unsigned char*)(hdr) + (unsigned)(hdr)->table + (unsigned)index*(hdr)->defsize)
#define GETENTRYNAME(hdr,entry) \
                        (char *)((unsigned char*)(hdr) + (unsigned)((AMX_FUNCSTUB*)(entry))->nameofs)

// Propagate any errors to caller
#define AMXERRORS(x) do {int a = (x); if (a != 0) return a;} while(0)

int loadprogram(const char *filename, char *error, size_t error_size)
{
    FIL file;
    FRESULT status = f_open(&file, filename, FA_READ);
    if (status != FR_OK)
    {
        snprintf(error, error_size, "Could not open file %s: %d", filename, status);
        return AMX_ERR_NOTFOUND;
    }
    
    /* Read file header */
    AMX_HEADER hdr;
    UINT read_count;
    f_read(&file, &hdr, sizeof hdr, &read_count);
    if (read_count != sizeof hdr || hdr.magic != AMX_MAGIC)
        return AMX_ERR_FORMAT;
    
    if ((hdr.flags & AMX_FLAG_OVERLAY) != 0) {
        snprintf(error, error_size, "Code overlays are not yet supported.");
        return AMX_ERR_OVERLAY;
    }
    
    if (hdr.stp > sizeof(vm_data))
        return AMX_ERR_MEMORY;
    
    /* Read the actual file, including the header (again) */
    f_lseek(&file, 0);
    f_read(&file, vm_data, hdr.size, &read_count);
    if (read_count != hdr.size)
        return AMX_ERR_FORMAT;
    
    memset(&amx, 0, sizeof(amx));
    AMXERRORS(amx_Init(&amx, vm_data));
    
    amxinit_display(&amx);
    amx_CoreInit(&amx);
    amxinit_string(&amx);
    amxinit_fixed(&amx);
    amxinit_wavein(&amx);
    amxinit_waveout(&amx);
    amxinit_menu(&amx);
    
    // Check that everything has been registered
    int regstat = amx_Register(&amx, NULL, -1);
    if (regstat != 0)
    {
        // Find out what is missing
        for (int i = 0; i < NUMENTRIES((AMX_HEADER*)amx.base,natives,libraries); i++)
        {
            AMX_FUNCSTUB *func = GETENTRY((AMX_HEADER*)amx.base,natives,i);
            if (func->address == 0)
            {
                snprintf(error, error_size, "Native function not found: %s",
                         GETENTRYNAME((AMX_HEADER*)amx.base,func));
                return AMX_ERR_NOTFOUND;
            }
        }
        
        snprintf(error, error_size, "Error registering native functions");
        return regstat;
    }
    
    return 0;
}

int doevents(AMX *amx)
{
    return amx_menu_doevents(amx);
}

#define REMAINING ((p < end) ? (end - p) : 0)

void show_pawn_traceback(const char *filename, AMX *amx, int return_status)
{
    AMX_HEADER *hdr = (AMX_HEADER*)amx->base;
    char buffer[500];
    char *p = buffer;
    char *end = buffer + 500;
    
    p += snprintf(p, REMAINING, "Virtual machine error: %s\n",
                  aux_StrError(return_status));
    
    p += snprintf(p, REMAINING, "CIP: %08lx PRI: %08lx ALT: %08lx\n\n",
                  amx->cip, amx->pri, amx->alt);
    
    {
        FIL file;
        AMX_DEBUG_INFO dbg;
        char tmp[40];
        bool have_dbg = false;
        if (!(hdr->flags & AMX_FLAG_DEBUG))
        {
            p += snprintf(p, REMAINING, "Symbolic debug info not available. "
                        "Please recompile with -d2.\n");
        }
        else
        {
            f_open(&file, filename, FA_READ);
            have_dbg = amxdbg_load(&file, &dbg);
            
            if (!have_dbg)
            {
                p += snprintf(p, REMAINING, "Failed to load symbolic debug info.\n");
            }
        }
        
        if (have_dbg)
        {
            if (amxdbg_find_location(&dbg, amx->cip, tmp, sizeof(tmp)))
            {
                p += snprintf(p, REMAINING, "Location: %s\n", tmp);
            }
            
            if (amxdbg_format_locals(&dbg, amx, amx->frm, amx->cip, tmp, sizeof(tmp)))
            {
                p += snprintf(p, REMAINING, "   %s\n", tmp);
            }
        }
        
        int i = 1;
        unsigned frame = amx->frm, caller;
        while (i < 4 && (caller = amxdbg_get_caller(amx, &frame)) != 0)
        {
            if (have_dbg && amxdbg_find_location(&dbg, caller, tmp, sizeof(tmp)))
            {
                p += snprintf(p, REMAINING, "Caller %d: %s\n", i, tmp);
            }
            else
            {
                p += snprintf(p, REMAINING, "Caller %d: %08x\n", i, caller);
            }
            
            if (have_dbg && amxdbg_format_locals(&dbg, amx, frame, caller, tmp, sizeof(tmp)))
            {
                p += snprintf(p, REMAINING, "   %s\n", tmp);
            }
            
            i++;
        }
        
        f_close(&file);
    }
    
    {
        FIL file;
        unsigned bytes;
        unsigned len = strlen(buffer);
        f_open(&file, "crash.txt", FA_WRITE | FA_CREATE_ALWAYS);
        f_write(&file, buffer, len, &bytes);
        f_close(&file);
        
        if (bytes == len)
        {
            p += snprintf(p, REMAINING, "\nThis message was written also to crash.txt.");
        }
    }
    
    show_msgbox("Program crashed", buffer);
}

#include "gpio.h"
DECLARE_GPIO(usart1_tx, GPIOA, 9);
DECLARE_GPIO(usart1_rx, GPIOA, 10);

void draw_app_menu();

int main(void)
{   
    __Set(BEEP_VOLUME, 0);
    // USART1 8N1 115200bps debug port
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
    USART1->BRR = ((72000000 / (16 * 115200)) << 4) | 1;
    USART1->CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE | USART_CR1_RXNEIE;
    gpio_usart1_tx_mode(GPIO_AFOUT_10);
    gpio_usart1_rx_mode(GPIO_HIGHZ_INPUT);
    
    // Reduce the wait states of the FPGA & LCD interface
    // It works for me, hopefully it works for you too :)
    FSMC_BTR1 = 0x10100110;
    FSMC_BTR2 = 0x10100110;
    
    __Set(ADC_CTRL, EN);       
    __Set(ADC_MODE, SEPARATE);
    
    debugf("Here!");
    
    delay_ms(500);
    
    printf("HEre!\n");
    
    FRESULT status = f_mount(0, &fatfs);
    while (status != FR_OK)
    {
        char buf[100];
        snprintf(buf, sizeof(buf), "Failed to open the FAT12 filesystem: "
                 "f_mount returned %d", status);
        show_msgbox("Filesystem error", buf);
        status = f_mount(0, &fatfs);
    }
    
    while (true)
    {
        char filename[13];
        select_file(filename);
        
        __Clear_Screen(0);
        
        char error[50] = {0};
        int status = loadprogram(filename, error, sizeof(error));
        if (status != 0)
        {
            char buffer[200];
            snprintf(buffer, sizeof(buffer),
                     "Loading of program %s failed:\n\n"
                     "Error %d: %s\n\n"
                     "%s\n", filename, status, aux_StrError(status), error);
            printf(buffer);
            printf(filename);
            show_msgbox("Program load failed", buffer);
        }
        else
        {
            draw_app_menu();
            
            cell ret;
            status = amx_Exec(&amx, &ret, AMX_EXEC_MAIN);
                
            while (status == AMX_ERR_SLEEP)
            {
                AMX nested_amx = amx;
                uint32_t end = get_time() + amx.pri;
                do {
                    status = doevents(&nested_amx);
                } while (get_time() < end && status == 0);
                
                if (status == 0)
                    status = amx_Exec(&amx, &ret, AMX_EXEC_CONT);
                else
                    amx = nested_amx; // Report errors properly
            }
            
            if (status != 0)
            {
                show_pawn_traceback(filename, &amx, status);
            }
            else
            {
                draw_menubar("Close", "", "", "");
                while (!get_keys(BUTTON1));
            }
        }
    }
    
    return 0;
}

