/* A message dialog, with just simply a Close button. */

#include "drawing.h"
#include "buttons.h"
#include "menubar.h"
#include "msgbox.h"
#include "BIOS.h"

void show_msgbox(const char *title, const char *message)
{
    show_question("Close", " ", title, " ", message);
}

int show_question(const char *opt1, const char *opt2, const char *opt3,
                   const char *opt4, const char *message)
{
    get_keys(ANY_KEY); // Clear key buffer
    __Clear_Screen(0);
    
    draw_menubar(opt1, opt2, opt3, opt4);
    
    draw_flowtext(message, 0, 0, 400, 200, RGB(255,255,255), 0, false);
    
    bool has_command[5] = {0, !!(*opt1), !!(*opt2), !!(*opt3), !!(*opt4)};
    int option = 0;
    while (!option)
    {
        if (get_keys(BUTTON1)) option = 1;
        if (get_keys(BUTTON2)) option = 2;
        if (get_keys(BUTTON3)) option = 3;
        if (get_keys(BUTTON4)) option = 4;
        
        if (!has_command[option]) option = 0;
    }
    
    __Clear_Screen(0);
    get_keys(ANY_KEY); // Clear key buffer
    
    return option;
}

