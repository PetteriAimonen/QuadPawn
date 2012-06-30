/** Functions to control the device hardware, such as screen backlight,
 * beeper and power saving.
 */

/// Set backlight level, value from 0 to 100 (0 is off).
native set_backlight(level);

/// Set FPGA & ADC power save mode.
/// To enter power save, simply call set_powersave(true).
/// To exit power save, call set_powersave(false) and then
/// reconfigure wavein_samplerate() etc. settings.
/// You'll also need to turn on the backlight using e.g. set_backlight(100).
native set_powersave(bool: enable);

/// Beep for the given time (in milliseconds), at the given
/// frequency (in hertz) and with the given volume (0 to 100).
/// Function will return immediately, beep will continue for given time.
native beep(milliseconds = 500, frequency = 400, volume = 80);

/// Returns the current battery voltage in millivolts.
/// Full is 4.2V, Empty is 3.5V, but discharge is not linear.
native battery_voltage();
