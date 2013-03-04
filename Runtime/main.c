#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "amx.h"
#include "amxpool.h"

#include "BIOS.h"
#include "stm32f10x.h"
#include "ds203_io.h"
#include "Interrupt.h"
#include "alterbios.h"
#include "ff.h"

#include "menubar.h"
#include "buttons.h"
#include "file_selector.h"
#include "amxaux.h"
#include "msgbox.h"
#include "amx_debug.h"
#include "amx_menu.h"

// For some reason, the headers don't have this register
#define FSMC_BTR1   (*((vu32 *)(0xA0000000+0x04)))
#define FSMC_BTR2   (*((vu32 *)(0xA0000008+0x04)))

AMX amx;
FIL amx_file;
char amx_filename[20];

// Data block allocated for the virtual machine
uint8_t vm_data[32768] __attribute__((aligned(4)));

int amxinit_display(AMX *amx);
int amx_CoreInit(AMX *amx);
int amxinit_string(AMX *amx);
int amxinit_fixed(AMX *amx);
int amxinit_wavein(AMX *amx);
int amxcleanup_wavein(AMX *amx);
int amxinit_waveout(AMX *amx);
int amxinit_file(AMX *amx);
int amxcleanup_file(AMX *amx);
int amxinit_buttons(AMX *amx);
int amxinit_fourier(AMX *amx);
int amxinit_time(AMX *amx);
int amxinit_device(AMX *amx);
int amxinit_fpga(AMX *amx);
int amx_timer_doevents(AMX *amx);
//cf usart test
int amxinit_usart(AMX *amx);


void overlay_init(AMX *amx, const char *filename, FIL *file);

#define AMX_ERR_ABORT 100
#define AMX_ERR_FILE_CHANGED 101

static int debughook(AMX *amx)
{
    return AMX_ERR_ABORT;
}

volatile bool ABORT = false;

void TimerTick()
{
    uint32_t ms;
    if (held_keys(ANY_KEY, &ms) == BUTTON4)
    {
        if (ms > 3000)
        {
            amx.debug = debughook;
        }
    
        if (ms > 3500)
        {
            ABORT = true;
        }
    }
    else if (held_keys(ANY_KEY, &ms) == (BUTTON3 | BUTTON4))
    {
        if (ms > 3000)
        {
            // It's quite ugly to do such a long task in an
            // interrupt handler. Furthermore, it may access
            // the filesystem at the wrong time.
            write_bitmap(select_filename("SSHOT%03d.BMP"), bmp_default_palette);
            
            __Set(BEEP_VOLUME, 20);
            while ((~__Get(KEY_STATUS)) & BUTTON4);
            __Set(BEEP_VOLUME, 0);
        }
    }
}

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
    FIL *file = &amx_file;
    FRESULT status = f_open(file, filename, FA_READ);
    if (status != FR_OK)
    {
        snprintf(error, error_size, "Could not open file %s: %d", filename, status);
        return AMX_ERR_NOTFOUND;
    }
    
    /* Read file header */
    memset(&amx, 0, sizeof(amx));
    AMX_HEADER hdr;
    UINT read_count;
    f_read(file, &hdr, sizeof hdr, &read_count);
    if (read_count != sizeof hdr || hdr.magic != AMX_MAGIC)
        return AMX_ERR_FORMAT;
    
    if (hdr.flags & AMX_FLAG_OVERLAY)
    {
        // Read the header
        f_lseek(file, 0);
        f_read(file, vm_data, hdr.cod, &read_count);
        
        // Read the data block
        f_lseek(file, hdr.dat);
        unsigned dat_size = hdr.hea - hdr.dat;
        f_read(file, vm_data + hdr.cod, dat_size, &read_count);
        if (read_count != dat_size)
            return AMX_ERR_FORMAT;
        
        unsigned static_size = (hdr.stp - hdr.dat) + hdr.cod;
        amx_poolinit(vm_data + static_size, sizeof(vm_data) - static_size);
        
        amx.base = vm_data;
        amx.data = vm_data + hdr.cod;
        overlay_init(&amx, amx_filename, &amx_file);
    }
    else
    {
        if (hdr.stp > sizeof(vm_data))
            return AMX_ERR_MEMORY;
        
        /* Read the actual file, including the header (again) */
        f_lseek(file, 0);
        f_read(file, vm_data, hdr.size, &read_count);
        if (read_count != hdr.size)
            return AMX_ERR_FORMAT;
    }
    
    AMXERRORS(amx_Init(&amx, vm_data));
    
    amxinit_display(&amx);
    amx_CoreInit(&amx);
    amxinit_string(&amx);
    amxinit_fixed(&amx);
    amxinit_wavein(&amx);
    amxinit_waveout(&amx);
    amxinit_menu(&amx);
    amxinit_file(&amx);
    amxinit_buttons(&amx);
    amxinit_fourier(&amx);
    amxinit_time(&amx);
    amxinit_device(&amx);
    amxinit_fpga(&amx);
    //cf
    amxinit_usart(&amx);
    
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
    int status = amx_menu_doevents(amx);
    if (status != 0)
        return status;
    
    status = amx_timer_doevents(amx);
    return status;
}

#define REMAINING ((p < end) ? (end - p) : 0)

const char *my_aux_StrError(int status)
{
    if (status == AMX_ERR_FILE_CHANGED)
        return "AMX file changed while running";
    else if (status == AMX_ERR_ABORT)
        return "Program aborted by user";
    else
        return aux_StrError(status);
}

void show_pawn_traceback(const char *filename, AMX *amx, int return_status)
{
    AMX_HEADER *hdr = (AMX_HEADER*)amx->base;
    char buffer[500];
    char *p = buffer;
    char *end = buffer + 500;
    
    p += snprintf(p, REMAINING, "Virtual machine error: %s\n",
                  my_aux_StrError(return_status));
    
    p += snprintf(p, REMAINING, "CIP: %08lx PRI: %08lx ALT: %08lx\n",
                  amx->cip, amx->pri, amx->alt);
    
    if (hdr->flags & AMX_FLAG_OVERLAY)
    {
        p += snprintf(p, REMAINING, "Current overlay index: %d\n", amx->ovl_index);
    }
    
    p += snprintf(p, REMAINING, "\n");
    
    {
        FIL *file = &amx_file;
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
            have_dbg = amxdbg_load(file, amx, &dbg);
            
            if (!have_dbg)
            {
                p += snprintf(p, REMAINING, "Failed to load symbolic debug info.\n");
            }
        }
        
        if (have_dbg)
        {
            unsigned location = amx->cip;
            
            if (hdr->flags & AMX_FLAG_OVERLAY)
            {
                location = (amx->cip << 16) | amx->ovl_index;
            }
            
            if (amxdbg_find_location(&dbg, location, tmp, sizeof(tmp)))
            {
                p += snprintf(p, REMAINING, "Location: %s\n", tmp);
            }
            
            if (amxdbg_format_locals(&dbg, amx, amx->frm, location, tmp, sizeof(tmp)))
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

int main(void)
{   
    __Set(BEEP_VOLUME, 0);
    
    // USART1 8N1 115200bps debug port
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
    USART1->BRR = 72000000 / 115200;
    USART1->CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE | USART_CR1_RXNEIE;
    gpio_usart1_tx_mode(GPIO_AFOUT_10);
    gpio_usart1_rx_mode(GPIO_HIGHZ_INPUT);
    NVIC_EnableIRQ(USART1_IRQn);
    NVIC_SetPriority(USART1_IRQn, 14); 
    
    printf("\nBoot! (cf3)\r\n");
    // Reduce the wait states of the FPGA & LCD interface
    // It works for me, hopefully it works for you too :)
    FSMC_BTR1 = 0x10100110;
    FSMC_BTR2 = 0x10100110;
    
    __Set(ADC_CTRL, EN);       
    __Set(ADC_MODE, SEPARATE);
    
    int status = alterbios_check();
    if (status < 0)
    {
        char buf[100];
        snprintf(buf, sizeof(buf), "AlterBIOS not found or too old: %d\n"
                 "Please install it from https://github.com/PetteriAimonen/AlterBIOS", status);
        while (1) show_msgbox("AlterBIOS is required", buf);
    }
    alterbios_init();
    
    get_keys(ALL_KEYS); // Clear key buffer
    
    //show_msgbox("CF", "Hello World");

    while (true)
    {
        select_file(amx_filename);
        
        get_keys(ANY_KEY);
        __Clear_Screen(0);
        
        char error[50] = {0};
        int status = loadprogram(amx_filename, error, sizeof(error));
        if (status != 0)
        {
            char buffer[200];
            snprintf(buffer, sizeof(buffer),
                     "Loading of program %s failed:\n\n"
                     "Error %d: %s\n\n"
                     "%s\n", amx_filename, status, my_aux_StrError(status), error);
            printf(buffer);
            printf(amx_filename);
            show_msgbox("Program load failed", buffer);
        }
        else
        {
            int idle_func = -1;
            if (amx_FindPublic(&amx, "@idle", &idle_func) != 0) idle_func = -1;
            
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
            
            if (status == 0 && idle_func != -1)
            {
                // Main() exited, keep running idle function.
                do {
                    status = doevents(&amx);
                    
                    if (status == 0)
                        status = amx_Exec(&amx, &ret, idle_func);
                } while (status == 0 && ret != 0);
            }
            
            amxcleanup_wavein(&amx);
            amxcleanup_file(&amx);
            
            if (status == AMX_ERR_EXIT && ret == 0)
                status = 0; // Ignore exit(0), but inform about e.g. exit(1)
            
            if (status != 0)
            {
                show_pawn_traceback(amx_filename, &amx, status);
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

