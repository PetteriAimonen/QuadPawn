/* A dialog for selecting a script from the internal filesystem */

#include <stdbool.h>
#include "ff.h"
#include "menubar.h"
#include "amx.h"
#include "drawing.h"
#include "BIOS.h"
#include "buttons.h"
#include "ds203_io.h"
#include <string.h>
#include <stdlib.h>
#include "msgbox.h"
#include "metadata.h"

extern FATFS fatfs;

static const uint32_t default_icon[] = {
0b000000000000001000000000000000,
0b000000000000011110000000000000,
0b000000000001110011100000000000,
0b000000000111000000111000000000,
0b000000011100000000001100000000,
0b000000110000000000000111000000,
0b000011100000000000000001110000,
0b001110000000000000000000011000,
0b011000000000000000000000000110,
0b110000000000000000000000000011,
0b101000000000000000000000000111,
0b110110000000000000000000001111,
0b101010100000000000000000111111,
0b110101011000000000000011111111,
0b101010101110000000001111111111,
0b110101010101100000111111111111,
0b101010101010101011111111111111,
0b110101010101010111111111111111,
0b101010101010101111111111111111,
0b110101010101010111111111111111,
0b001110101010101111111111111100,
0b000011010101010111111111110000,
0b000001101010101111111111100000,
0b000000011101010111111110000000,
0b000000000110101111111000000000,
0b000000000001010111110000000000,
0b000000000000111111000000000000,
0b000000000000001100000000000000
};

#define DEFAULT_ICON_HEIGHT (sizeof(default_icon)/4)

#define SLOT_W 100
#define SLOT_H 70
#define ICONS_ON_SCREEN 12

static void draw_item(int index, const uint32_t *icon, int icon_height,
                      const char *line1, const char *line2)
{
    int x = (index % 4) * SLOT_W;
    int y = 220 - (index / 4 + 1) * SLOT_H;
    
    draw_bitmap(icon, x + 34, y + 30, RGB(255,255,255), icon_height, true);
    draw_flowtext(line1, x, y + 2, SLOT_W, 28, RGB(255,255,255), 0, true);
}

static bool is_a_script(const char *filename)
{
    while (*filename && *filename != '.') filename++;
    
    return filename[0] == '.' && (filename[1] | 0x20) == 'a' &&
           (filename[2] | 0x20) == 'm' && (filename[3] | 0x20) == 'x';
}

static bool seek_dir(int page, DIR *dir, FILINFO *file)
{
    while (page--)
    {
        for (int i = 0; i < ICONS_ON_SCREEN;)
        {
            if (f_readdir(dir, file) != 0 || file->fname[0] == 0)
                return false;
            
            if (is_a_script(file->fname))
                i++;
        }
    }
    return true;
}

static void render_screen(int page, int *maxindex)
{
    DIR dir;
    FILINFO file;
    
    __Clear_Screen(0);
    draw_menubar("Run", "Refresh", "", "About");
    
    f_opendir(&dir, "/");
    if (!seek_dir(page, &dir, &file))
    {
        debugf("Directory seek failed");
        return;
    }
    
    int i;
    for (i = 0; i < ICONS_ON_SCREEN; )
    {
        if (f_readdir(&dir, &file) != 0 || file.fname[0] == 0)
            break;
        
        if (is_a_script(file.fname))
        {
            uint32_t icon_buf[32];
            const uint32_t *icon = default_icon;
            int icon_size = 0;
            char name[20] = {0};
            
            if (get_program_metadata(file.fname, icon_buf, &icon_size, name, sizeof(name)))
            {
                if (icon_size != 0)
                {
                    icon = icon_buf;
                }
            }
            
            if (icon == default_icon)
                icon_size = DEFAULT_ICON_HEIGHT;
            
            if (name[0] == 0)
                memcpy(name, file.fname, 13);
            
            draw_item(i, icon, icon_size, name, "");
            i++;
        }
    }
    
    if (f_readdir(&dir, &file) == 0 && file.fname[0] != 0)
        i++;
    
    *maxindex = i;
    
    if (*maxindex == 0)
    {
        draw_text("Please copy some .AMX files to the USB drive :)", 200, 100, 0xFFFF, 0, true);
    }
}

#include <stdio.h>

static bool get_name(char dest[13], int page, int index)
{
    DIR dir;
    FILINFO file;
    
    f_opendir(&dir, "/");
    if (!seek_dir(page, &dir, &file))
    {
        debugf("Directory seek failed");
        return false;
    }
    
    for (int i = 0; i <= index; )
    {
        if (f_readdir(&dir, &file) != 0 || file.fname[0] == 0)
            return false;
        
        if (is_a_script(file.fname))
            i++;
    }
    
    printf("Selected %s\n", file.fname);
    memcpy(dest, file.fname, 13);
    return true;
}

static void show_cursor(int index, int color)
{
    if (index < 0) return;
    
    int x = (index % 4) * SLOT_W;
    int y = 219 - (index / 4 + 1) * SLOT_H;
    
    draw_rectangle(x, y, SLOT_W, SLOT_H - 2, color, 0);
}

void select_file(char result[13])
{
    static int page = 0; // Remember selections while the program runs
    static int index = 0;
    int maxindex = 0;
    
    bool rerender = true;
    
    for(;;)
    {
        if (get_keys(BUTTON1) && index >= 0)
        {
            // File selected, get the name and return
            if (get_name(result, page, index))
                return;
            else
                rerender = true; // Couldn't find file, maybe fs has changed?
        }
        
        if (get_keys(BUTTON2))
        {
            // Refresh
            f_flush(&fatfs);
            rerender = true;
        }
        
        if (get_keys(BUTTON4))
        {
            show_msgbox("About PAWN_APP",
                        "Pawn scripting language for DSO Quad\n"
                        "(C) 2012 Petteri Aimonen <jpa@kapsi.fi>\n"
                        "\n"
                        "Built " __DATE__ " at " __TIME__ "\n"
                        "Git id " COMMITID "\n"
                        "GCC version " __VERSION__
            );
            rerender = true;
        }
        
        if (peek_keys(SCROLL1_LEFT | SCROLL1_RIGHT | SCROLL2_LEFT | SCROLL2_RIGHT))
        {
            // Clear old cursor
            show_cursor(index, 0);
            
            // Update index
            if (get_keys(SCROLL1_LEFT)) index -= 4;
            if (get_keys(SCROLL1_RIGHT)) index += 4;
            if (get_keys(SCROLL2_LEFT)) index -= 1;
            if (get_keys(SCROLL2_RIGHT)) index += 1;
            
            if (index < 0)
            {
                if (page > 0)
                {
                    page--;
                    render_screen(page, &maxindex);
                    index += 12;
                }
                else
                {
                    index = 0;
                }
            }
            
            if (index >= maxindex)
            {
                if (maxindex > ICONS_ON_SCREEN)
                {
                    page++;
                    index -= 12;
                    rerender = true;
                }
                else
                {
                    index = maxindex - 1;
                }
            }
            
            // Draw new cursor
            show_cursor(index, RGB(128, 128, 255));
        }
        
        if (rerender)
        {
            render_screen(page, &maxindex);
            show_cursor(index, RGB(128, 128, 255));
            rerender = false;
            
            if (index >= maxindex)
                index = maxindex - 1;
        }
    }
}
