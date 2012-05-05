/* Time functions based on the 1ms tick. */

#include "buttons.h"
#include "amx.h"

static cell AMX_NATIVE_CALL amx_get_time(AMX *amx, const cell *params)
{
    return get_time();
}

static cell AMX_NATIVE_CALL amx_delay_ms(AMX *amx, const cell *params)
{
    delay_ms(params[1]);
    return 0;
}

static uint32_t timer_period = 0;
static uint32_t timer_last_tick;
static int timer_func = -1;

static cell AMX_NATIVE_CALL amx_set_timer(AMX *amx, const cell *params)
{
    timer_period = params[1];
    timer_last_tick = get_time();
    return 0;
}

int amx_timer_doevents(AMX *amx)
{
    int status;
    cell retval;
    
    if (timer_func != -1 && timer_period != 0 &&
        get_time() - timer_last_tick >= timer_period)
    {
        status = amx_Exec(amx, &retval, timer_func);
        return status;
    }
    
    return 0;
}

int amxinit_time(AMX *amx)
{
    static const AMX_NATIVE_INFO funcs[] = {
        {"get_time", amx_get_time},
        {"delay_ms", amx_delay_ms},
        {"set_timer", amx_set_timer},
        {0, 0}
    };
    
    if (amx_FindPublic(amx, "@timertick", &timer_func) != 0)
        timer_func = -1;
    
    return amx_Register(amx, funcs, -1);
}