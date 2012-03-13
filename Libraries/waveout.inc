/** Functions for generating waveforms for the wave out port
  * These can use either the TIM4 for generating digital waveforms or
  * the DAC for generating analog waves.
  */

#include <fixed>
#include <core>

/// Conversion factor from volts to DAC value.
new Fixed:waveout_scale = FIX(1462.5); // 4095/2.8V

/// Configure the device for analog waveout and start feeding samples from
/// the buffer to the output. Maximum samplerate is nominally 1MHz.
/// Returns the actual samplerate (rounded to nearest smaller value).
native waveout_analog(buffer[], samplerate, count = sizeof buffer);

/// Set digital (square wave) output, with adjustable duty cycle.
/// Returns the actual frequency.
native waveout_digital(frequency, Fixed: duty = FIX(0.5));

/// Set the waveout to generate a constant voltage
stock waveout_voltage(Fixed: voltage)
{
    static buffer[1];
    buffer[0] = fround(voltage * waveout_scale);
    waveout_analog(buffer, 1);
}

/// Helper function that sets up a temp buffer so that we don't emit spurious
/// stuff while updating the waveform. Also, returns the real samplerate.
static stock waveout_temporary(samplerate, offset)
{
    // Max. samplerate for accurate output freq seems to be about 4MHz.
    // At samplerates higher than this, the DMA or the DAC starts to lag
    // and the actual samplerate is lower than what we try to set.
    const max_samplerate = 4000000;
    if (samplerate > max_samplerate) samplerate = max_samplerate;

    // Use a temp buffer so that we don't emit spurious stuff while updating
    // the waveform. Also, this lets us determine the real samplerate.
    static tmp[1];
    tmp[0] = offset;
    samplerate = waveout_analog(tmp, samplerate);
    return samplerate;
}

/// Set the waveout to generate sine wave at the given frequency.
/// You need to provide a buffer that is used to store and play back
/// the waveform. The buffer must be valid as long as you want the
/// signal to play. The longer the buffer, the more accurate the
/// waveform will be. Returns the samplerate used.
stock waveout_sinewave(buffer[], frequency, amplitude = 2000, offset = 2047,
                       &realbufsize = 0, bufsize = sizeof buffer)
{
    new samplerate = frequency * bufsize;
    samplerate = waveout_temporary(samplerate, offset);

    // Try to have a whole number of repeats in the buffer to avoid glitches
    new repeats = bufsize * frequency / samplerate;
    realbufsize = repeats * samplerate / frequency;

    // Now generate a sine table using cos(2 pi f * i / samplerate)
    // Uses cos() instead of sin() to work properly even when ratio = 2
    // The intermediate multiplier is scaled by 4096 to increase precision.
    new Fixed: ratio = (Fixed: samplerate) / (Fixed: frequency);
    new Fixed: multiplier = (2 * pi * 4096 / ratio);
    for (new i = 0; i < realbufsize; i++)
    {
        buffer[i] = fround(cos(multiplier * Fixed:(i * 16)) * amplitude + offset);
        buffer[i] = clamp(buffer[i], 0, 4095);
    }

    // And set the table for the waveout
    return waveout_analog(buffer, samplerate, realbufsize);
}

/// Set the waveout to generate triangle wave
stock waveout_triangle(buffer[], frequency, amplitude = 2000, offset = 2047,
                       Fixed: duty = FIX(0.5), &realbufsize = 0, bufsize = sizeof buffer)
{
    new samplerate = frequency * bufsize;
    samplerate = waveout_temporary(samplerate, offset);
    
    // Try to have a whole number of repeats in the buffer to avoid glitches
    new repeats = bufsize * frequency / samplerate;
    realbufsize = repeats * samplerate / frequency;

    // Ratio is samples per cycle
    new Fixed: ratio = (Fixed: samplerate) / (Fixed: frequency);
    new Fixed: upramp = fixed(amplitude * 2) / (duty * ratio);
    new Fixed: downramp = fixed(amplitude * 2) / ((FIX(1.0) - duty) * ratio);
    new Fixed: corner = duty * ratio;
    for (new i = 0; i < realbufsize; i++)
    {
        new Fixed: pos = fixed(i) % ratio;
        if (pos <= corner)
            buffer[i] = fround(upramp * pos);
        else
            buffer[i] = offset + amplitude - fround(downramp * (pos - corner));
    }
    
    return waveout_analog(buffer, samplerate, realbufsize);
}

