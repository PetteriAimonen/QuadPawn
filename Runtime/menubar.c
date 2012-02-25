#include "drawing.h"
#include "menubar.h"
#include "BIOS.h"
#include "string.h"
#include <stdio.h>

#define MENU_BG         RGB(0, 0, 60)
#define MENU_BORDER     RGB(0, 0, 120)
#define MENU_TEXT       RGB(255,255,255)

static void draw_batteryicon();

void draw_menubar(const char *btn1, const char *btn2, const char *btn3, const char *btn4)
{
    __Set(BETTERY_DT, 1); // Start battery voltage measurement
    
    fill_rectangle(0, 220, 400, 20, MENU_BG);
    drawline(0, 220, 400, 220, MENU_BORDER, 0);
    drawline(0, 240, 400, 240, MENU_BORDER, 0);
    
    draw_text(btn1, 20, 222, MENU_TEXT, -1, true);
    draw_text(btn2, 87, 222, MENU_TEXT, -1, true);
    draw_text(btn3, 155, 222, MENU_TEXT, -1, true);
    draw_text(btn4, 224, 222, MENU_TEXT, -1, true);
    
    draw_batteryicon();
}


static const uint32_t battery_icon[10] = {
    0b011111111111111111110000,
    0b100000000000000000001000,
    0b100000000000000000001000,
    0b100000000000000000001100,
    0b100000000000000000000010,
    0b100000000000000000000010,
    0b100000000000000000001100,
    0b100000000000000000001000,
    0b100000000000000000001000,
    0b011111111111111111110000
};

static const uint32_t fillpat1 = 0b101010101010101010101000;
static const uint32_t fillpat2 = 0b010101010101010101011000;

static void draw_batteryicon()
{
    // Get battery charge level in percents
    int level = __Get(V_BATTERY);
    level = (level - 3500) * 100 / 500;
    
    // Copy the icon base
    uint32_t icon[10];
    memcpy(icon, battery_icon, sizeof(icon));
    
    // Fill in the charge level
    int shift = 20 - 20 * level / 100;
    if (shift < 0) shift = 0;
    uint32_t p1 = fillpat1 << shift;
    uint32_t p2 = fillpat2 << shift;
    
    for (int i = 1; i < 9; i += 2)
    {
        icon[i] |= p1;
        icon[i + 1] |= p2;
    }
    
    draw_bitmap(icon, 370, 224, MENU_TEXT, 10, false);
}
