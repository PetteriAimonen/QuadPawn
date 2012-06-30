/* Device backlight etc. */

#include "amx.h"
#include "buttons.h"
#include "BIOS.h"
#include <stm32f10x.h>

static cell AMX_NATIVE_CALL amx_set_backlight(AMX *amx, const cell *params)
{
    __Set(BACKLIGHT, params[1]);
    return 0;
}

static cell AMX_NATIVE_CALL amx_set_powersave(AMX *amx, const cell *params)
{
    if (params[1])
    {
        __Set(BACKLIGHT, 0);
        __Set(STANDBY, 1);
    }
    else
    {
        __Set(STANDBY, 0);
    }
    
    return 0;
}

static cell AMX_NATIVE_CALL amx_beep(AMX *amx, const cell *params)
{
    // beep(milliseconds = 500, frequency = 400, volume = 80);
    
    int psc = 72000000 / 100 / params[2];
    if (psc > 7200) psc = 7200;
    if (psc < 36) psc = 36;
    
    TIM8->PSC = psc;
    
    beep(params[1], params[3]);
    return 0;
}

static cell AMX_NATIVE_CALL amx_battery_voltage(AMX *amx, const cell *params)
{
    __Set(BETTERY_DT, 1); // Start battery voltage measurement
    delay_ms(1);
    return __Get(V_BATTERY);
}

int amxinit_device(AMX *amx)
{
    static const AMX_NATIVE_INFO funcs[] = {
        {"set_backlight", amx_set_backlight},
        {"set_powersave", amx_set_powersave},
        {"beep", amx_beep},
        {"battery_voltage", amx_battery_voltage},
        {0, 0}
    };
    
    return amx_Register(amx, funcs, -1);
}
