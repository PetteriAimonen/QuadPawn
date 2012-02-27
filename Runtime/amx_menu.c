/* Pawn functions for controlling the menu bar. */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "menubar.h"
#include "buttons.h"
#include "amx.h"

typedef struct {
    int16_t b1_func, b2_func, b3_func;
    char b1_label[8], b2_label[8], b3_label[8];
    int16_t s1_func, s2_func;
} menuconf_t;

static menuconf_t configs[4];
static int current_config = 0;
#define MAX_CONFIG 3

static void clear_conf()
{
    menuconf_t *conf = configs + current_config;
    memset(conf, 0, sizeof(menuconf_t));
    conf->b1_func = conf->b2_func = conf->b3_func = -1;
    conf->s1_func = conf->s2_func = -1;
}

void draw_app_menu()
{
    menuconf_t *conf = configs + current_config;
    draw_menubar(conf->b1_label, conf->b2_label, conf->b3_label, "System");
}

static bool setfunc(AMX *amx, int16_t *variable, cell *param)
{
    char *funcname;
    amx_StrParam(amx, param, funcname);
    if (funcname[0] == 0)
    {
        *variable = -1;
        return true;
    }
    
    int tmp;
    if (amx_FindPublic(amx, funcname, &tmp) != 0)
    {
        amx_RaiseError(amx, AMX_ERR_NOTFOUND);
        return false;
    }
    *variable = tmp;
    return true;
}

static cell AMX_NATIVE_CALL amx_set_buttons(AMX *amx, const cell *params)
{
    menuconf_t *conf = configs + current_config;
    
    if (!setfunc(amx, &conf->b1_func, (cell*)params[2])) return false;
    amx_GetString(conf->b1_label, (cell*)params[1], 0, sizeof(conf->b1_label));
    
    if (!setfunc(amx, &conf->b2_func, (cell*)params[4])) return false;
    amx_GetString(conf->b2_label, (cell*)params[3], 0, sizeof(conf->b2_label));
    
    if (!setfunc(amx, &conf->b3_func, (cell*)params[6])) return false;
    amx_GetString(conf->b3_label, (cell*)params[5], 0, sizeof(conf->b3_label));
    
    draw_app_menu();
    return 0;
}

static cell AMX_NATIVE_CALL amx_redraw_menu(AMX *amx, const cell *params)
{
    draw_app_menu();
    return 0;
}

int amx_menu_doevents(AMX *amx)
{
    if (!peek_keys(BUTTON1 | BUTTON2 | BUTTON3))
        return 0;
    
    menuconf_t *conf = configs + current_config;
    int status;
    cell retval;
    
    if (get_keys(BUTTON1) && conf->b1_func != -1)
    {
        status = amx_Exec(amx, &retval, conf->b1_func);
        if (status != 0) return status;
    }
    
    if (get_keys(BUTTON2) && conf->b2_func != -1)
    {
        status = amx_Exec(amx, &retval, conf->b2_func);
        if (status != 0) return status;
    }
    
    if (get_keys(BUTTON3) && conf->b3_func != -1)
    {
        status = amx_Exec(amx, &retval, conf->b3_func);
        if (status != 0) return status;
    }
    
    return 0;
}

int amxinit_menu(AMX *amx)
{
    static const AMX_NATIVE_INFO funcs[] = {
        {"set_buttons", amx_set_buttons},
        {"redraw_menu", amx_redraw_menu},
        {0, 0}
    };
    
    clear_conf();
    
    return amx_Register(amx, funcs, -1);
}
