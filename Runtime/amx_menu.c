/* Pawn functions for controlling the menu bar. */

#include <stdbool.h>
#include <string.h>
#include "menubar.h"
#include "amx.h"

// The current functions and labels set for the buttons
static int b1_function = -1, b2_function = -1, b3_function = -1;
static char b1_label[8], b2_label[8], b3_label[8];

static int s1_function = -1, s2_function = -1;
static int sp1_function = -1, sp2_function = -1;

void draw_app_menu()
{
    draw_menubar(b1_label, b2_label, b3_label, "System");
}

static bool setfunc(AMX *amx, int *variable, cell *param)
{
    char *funcname;
    amx_StrParam(amx, param, funcname);
    if (amx_FindPublic(amx, funcname, variable) != 0)
    {
        amx_RaiseError(amx, AMX_ERR_NATIVE);
        return false;
    }
    return true;
}

static cell AMX_NATIVE_CALL amx_set_button1(AMX *amx, const cell *params)
{
    // set_button1(label, function)
    if (!setfunc(amx, &b1_function, (cell*)params[2])) return false;
    amx_GetString(b1_label, (cell*)params[1], 0, sizeof(b1_label));
    draw_app_menu();
    return 0;
}

static cell AMX_NATIVE_CALL amx_set_button2(AMX *amx, const cell *params)
{
    if (!setfunc(amx, &b2_function, (cell*)params[2])) return false;
    amx_GetString(b2_label, (cell*)params[1], 0, sizeof(b2_label));
    draw_app_menu();
    return 0;
}

static cell AMX_NATIVE_CALL amx_set_button3(AMX *amx, const cell *params)
{
    if (!setfunc(amx, &b3_function, (cell*)params[2])) return false;
    amx_GetString(b3_label, (cell*)params[1], 0, sizeof(b3_label));
    draw_app_menu();
    return 0;
}

static cell AMX_NATIVE_CALL amx_redraw_menu(AMX *amx, const cell *params)
{
    draw_app_menu();
    return 0;
}

int amxinit_menu(AMX *amx)
{
    static const AMX_NATIVE_INFO funcs[] = {
        {"set_button1", amx_set_button1},
        {"set_button2", amx_set_button2},
        {"set_button3", amx_set_button3},
        {"redraw_menu", amx_redraw_menu},
        {0, 0}
    };
    
    b1_function = b2_function = b3_function = s1_function = s2_function = sp1_function = sp2_function = -1;
    b1_label[0] = b2_label[0] = b3_label[0] = 0;
    
    return amx_Register(amx, funcs, -1);
}
