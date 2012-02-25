/* A message dialog, with just simply a Close button. */

#include "drawing.h"
#include "buttons.h"
#include "menubar.h"
#include "BIOS.h"

void show_msgbox(const char *title, const char *message)
{
    __Clear_Screen(0);
    
    draw_menubar("Close", "", title, "");
    
    draw_flowtext(message, 0, 0, 400, 200, RGB(255,255,255), 0, false);
    
    while (!get_keys(BUTTON1));
    
    __Clear_Screen(0);
    
    return;
}

