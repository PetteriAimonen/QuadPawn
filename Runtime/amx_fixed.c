/* Bindings from PAWN to libfixmath */

#include "amx.h"
#include "fix16.h"

static cell AMX_NATIVE_CALL amx_fadd(AMX *amx, const cell *params)
{
    return fix16_sadd(params[1], params[2]);
}

static cell AMX_NATIVE_CALL amx_fsub(AMX *amx, const cell *params)
{
    return fix16_ssub(params[1], params[2]);
}

static cell AMX_NATIVE_CALL amx_fmul(AMX *amx, const cell *params)
{
    return fix16_smul(params[1], params[2]);
}

static cell AMX_NATIVE_CALL amx_fdiv(AMX *amx, const cell *params)
{
    return fix16_sdiv(params[1], params[2]);
}

static cell AMX_NATIVE_CALL amx_fixed(AMX *amx, const cell *params)
{
    return fix16_from_int(params[1]);
}

static cell AMX_NATIVE_CALL amx_fround(AMX *amx, const cell *params)
{
    return fix16_to_int(params[1]);
}

static cell AMX_NATIVE_CALL amx_exp(AMX *amx, const cell *params)
{
    return fix16_exp(params[1]);
}

static cell AMX_NATIVE_CALL amx_log(AMX *amx, const cell *params)
{
    return fix16_log(params[1]);
}

static cell AMX_NATIVE_CALL amx_sqrt(AMX *amx, const cell *params)
{
    return fix16_sqrt(params[1]);
}

static cell AMX_NATIVE_CALL amx_sin(AMX *amx, const cell *params)
{
    return fix16_sin(params[1]);
}

static cell AMX_NATIVE_CALL amx_atan2(AMX *amx, const cell *params)
{
    return fix16_atan2(params[1], params[2]);
}

int amxinit_fixed(AMX *amx)
{
    static const AMX_NATIVE_INFO funcs[] = {
        {"fadd", amx_fadd},
        {"fsub", amx_fsub},
        {"fmul", amx_fmul},
        {"fdiv", amx_fdiv},
        {"fixed", amx_fixed},
        {"fround", amx_fround},
        {"exp", amx_exp},
        {"log", amx_log},
        {"sqrt", amx_sqrt},
        {"sin", amx_sin},
        {"atan2", amx_atan2},
        {0, 0}
    };
    
    return amx_Register(amx, funcs, -1);
}
