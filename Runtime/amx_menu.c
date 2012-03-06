/* Pawn functions for controlling the menu bar. */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "menubar.h"
#include "buttons.h"
#include "amx.h"
#include "amx_menu.h"
#include "ds203_io.h"

int b1_func, b2_func, b3_func, b4_func;

static cell AMX_NATIVE_CALL amx_draw_menubar(AMX *amx, const cell *params)
{
    char *b1, *b2, *b3, *b4;
    amx_StrParam(amx, params[1], b1);
    amx_StrParam(amx, params[2], b2);
    amx_StrParam(amx, params[3], b3);
    amx_StrParam(amx, params[4], b4);
    
    draw_menubar(b1, b2, b3, b4);
    return 0;
}

int amx_menu_doevents(AMX *amx)
{
    if (!peek_keys(BUTTON1 | BUTTON2 | BUTTON3 | BUTTON4))
        return 0;

    int status;
    cell retval;
    
    if (b1_func != -1 && get_keys(BUTTON1))
    {
        status = amx_Exec(amx, &retval, b1_func);
        if (status != 0) return status;
    }
    
    if (b2_func != -1 && get_keys(BUTTON2))
    {
        status = amx_Exec(amx, &retval, b2_func);
        if (status != 0) return status;
    }
    
    if (b3_func != -1 && get_keys(BUTTON3))
    {
        status = amx_Exec(amx, &retval, b3_func);
        if (status != 0) return status;
    }
    
    if (b3_func != -1 && get_keys(BUTTON4))
    {
        status = amx_Exec(amx, &retval, b4_func);
        if (status != 0) return status;
    }
    
    return 0;
}

int amxinit_menu(AMX *amx)
{
    static const AMX_NATIVE_INFO funcs[] = {
        {"draw_menubar", amx_draw_menubar},
        {0, 0}
    };
    
    if (amx_FindPublic(amx, "@button1", &b1_func) != 0)
        b1_func = -1;
    
    if (amx_FindPublic(amx, "@button2", &b2_func) != 0)
        b2_func = -1;
    
    if (amx_FindPublic(amx, "@button3", &b3_func) != 0)
        b3_func = -1;
    
    if (amx_FindPublic(amx, "@button4", &b4_func) != 0)
        b4_func = -1;
    
    return amx_Register(amx, funcs, -1);
}
