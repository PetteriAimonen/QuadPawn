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

int amxinit_time(AMX *amx)
{
    static const AMX_NATIVE_INFO funcs[] = {
        {"get_time", amx_get_time},
        {"delay_ms", amx_delay_ms},
        {0, 0}
    };
    
    return amx_Register(amx, funcs, -1);
}