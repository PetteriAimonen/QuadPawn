/* Polling interface to buttons */

#include "amx.h"
#include "buttons.h"

static cell AMX_NATIVE_CALL amx_peek_keys(AMX *amx, const cell *params)
{
    return peek_keys(params[1]);
}

static cell AMX_NATIVE_CALL amx_get_keys(AMX *amx, const cell *params)
{
    return get_keys(params[1]);
}

static cell AMX_NATIVE_CALL amx_held_keys(AMX *amx, const cell *params)
{
    return held_keys(params[1], (uint32_t*)params[2]);
}

static cell AMX_NATIVE_CALL amx_scroller_speed(AMX *amx, const cell *params)
{
    return scroller_speed();
}

int amxinit_buttons(AMX *amx)
{
    static const AMX_NATIVE_INFO funcs[] = {
        {"peek_keys", amx_peek_keys},
        {"get_keys", amx_get_keys},
        {"held_keys", amx_held_keys},
        {"scroller_speed", amx_scroller_speed},
        {0, 0}
    };
    
    return amx_Register(amx, funcs, -1);
}
