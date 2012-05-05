/** Calibration constants for converting measured values into volts. */

#include <fixed>
#include <file>
#include <inifile>

/// Table indexed by [range][channel * 3 + N] where
/// N=0: Measured ADC value for ground when offset is set as 0
/// N=1: when set as 128 and N=2: when set as 256
new Fixed: calibration_offsets[8][6] = [
    [FIX(0.0), FIX(128.0), FIX(256.0), FIX(0.0), FIX(128.0), FIX(256.0)],
    [FIX(0.0), FIX(128.0), FIX(256.0), FIX(0.0), FIX(128.0), FIX(256.0)], 
    [FIX(0.0), FIX(128.0), FIX(256.0), FIX(0.0), FIX(128.0), FIX(256.0)], 
    [FIX(0.0), FIX(128.0), FIX(256.0), FIX(0.0), FIX(128.0), FIX(256.0)], 
    
    [FIX(0.0), FIX(128.0), FIX(256.0), FIX(0.0), FIX(128.0), FIX(256.0)], 
    [FIX(0.0), FIX(128.0), FIX(256.0), FIX(0.0), FIX(128.0), FIX(256.0)], 
    [FIX(0.0), FIX(128.0), FIX(256.0), FIX(0.0), FIX(128.0), FIX(256.0)], 
    [FIX(0.0), FIX(128.0), FIX(256.0), FIX(0.0), FIX(128.0), FIX(256.0)]
];

/// Table indexed by [range][channel]. Value is ADC units per volt.
new Fixed: calibration_scales[8][2] = [
    [FIX(500.0), FIX(500.0)], // 50mV
    [FIX(250.0), FIX(250.0)], // 100mV
    [FIX(125.0), FIX(125.0)], // 200mV
    [FIX(50.0),  FIX(50.0)], // 500mV
    [FIX(25.0),  FIX(25.0)], // 1V
    [FIX(12.5),  FIX(12.5)], // 2V
    [FIX(5.0),   FIX(5.0)], // 5V
    [FIX(2.5),   FIX(2.5)]  // 10V
];

stock bool:save_calibration()
{
    new File: f = f_open("calibrat.ini", FA_WRITE | FA_CREATE_ALWAYS);
    if (!f) return false;
    
    new optname{32};
    for (new i = 0; i < 8; i++)
    {
        optname = "Offsets_";
        strcat(optname, adc_range_names[i]);
        inifile_writefarray(f, optname,  calibration_offsets[i]);
    }
    
    for (new i = 0; i < 8; i++)
    {
        optname = "Scales_";
        strcat(optname, adc_range_names[i]);
        inifile_writefarray(f, optname,   calibration_scales[i]);
    }
    
    return f_close(f);
}

stock bool:load_calibration()
{
    new File: f = f_open("calibrat.ini", FA_READ);
    if (!f) return false;
    
    new optname{32};
    for (new i = 0; i < 8; i++)
    {
        optname = "Offsets_";
        strcat(optname, adc_range_names[i]);
        inifile_getfarray(f, optname,  calibration_offsets[i]);
    }
    
    for (new i = 0; i < 8; i++)
    {
        optname = "Scales_";
        strcat(optname, adc_range_names[i]);
        inifile_getfarray(f, optname,   calibration_scales[i]);
    }
    
    return f_close(f);
}

// Get the calibration data for the currently active channel configuration.
// Then the voltage can be found out as (value - bias) / scale
stock get_calibration(Channel: channel, &Fixed: bias, &Fixed: scale)
{
    new ADCCoupling: coupling, ADCRange: range, offset;
    
    if (channel == Ch_A)
        getconfig_chA(coupling, range, offset);
    else
        getconfig_chB(coupling, range, offset);
    
    new idx = (channel == Ch_A) ? 0 : 1;
    new Fixed: value_0 = calibration_offsets[range][idx * 3];
    new Fixed: value_128 = calibration_offsets[range][idx * 3 + 1];
    new Fixed: value_256 = calibration_offsets[range][idx * 3 + 2];
    
    if (offset < 128)
    {
        bias = interpolate(offset, 0, value_0, 128, value_128);
    }
    else
    {
        bias = interpolate(offset, 128, value_128, 256, value_256);
    }
    
    scale = calibration_scales[range][idx];
}

/// Convert a measured value from ADC to voltage.
stock Fixed: get_voltage(Channel: channel, Fixed:value)
{
    new Fixed: bias, Fixed: scale;
    get_calibration(channel, bias, scale);
    return (value - bias) / scale;
}

