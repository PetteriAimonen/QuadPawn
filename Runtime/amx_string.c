#include <stdbool.h>
#include <stdlib.h>
#include "amx.h"

static cell AMX_NATIVE_CALL amx_strlen(AMX *amx, const cell *params)
{
    int result = 0;
    amx_StrLen((cell*)params[1], &result);
    return result;
}

static cell AMX_NATIVE_CALL amx_strval(AMX *amx, const cell *params)
{
    // strval(const string[], index, endindex, base);
    char *text;
    amx_StrParam(amx, params[1], text);
    char *start = text + params[2];
    int base = params[4];
    
    char *endptr;
    int result = strtol(text, &endptr, base);
    *((cell*)params[3]) = endptr - text;
    return result;
}

static cell AMX_NATIVE_CALL amx_valstr(AMX *amx, const cell *params)
{
    // valstr(dest[], value, maxlength = sizeof dest, base = 10, minlength = 0);
    char dest[36];
    int value = params[2];
    int base = params[4];
    int minlength = params[5];
    int count = 0;
    
    dest[35] = 0;
    char *p = dest + 34;
    int maxlength = 32;
    
    bool negative = (value < 0);
    if (negative) value = -value;
    
    if (value == 0)
    {
        *p-- = '0';
        count++;
    }
    else
    {
        while (value && count < maxlength)
        {
            int digit = value % base;
            value /= base;
            
            *p-- = (digit < 10) ? (digit + '0') : (digit - 10 + 'A');
            count++;
        }
    }
    
    while (count < minlength)
    {
        *p-- = '0';
        count++;
    }

    if (negative)
    {
        *p-- = '-';
        count++;
    }
        
    amx_SetString((cell*)params[1], p+1, true, false, params[3]);
    return count;
}

int amxinit_string(AMX *amx)
{
    static const AMX_NATIVE_INFO funcs[] = {
        {"strlen", amx_strlen},
        {"strval", amx_strval},
        {"valstr", amx_valstr},
        {0, 0}
    };
    
    return amx_Register(amx, funcs, -1);
}
