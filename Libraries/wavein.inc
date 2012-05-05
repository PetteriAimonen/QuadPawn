/** Functions for reading input signals from the ADC / digital inputs. */

const ADCCoupling: {
    ADC_DC = 0,
    ADC_AC = 1
}

const ADCRange: {
    ADC_50mV = 0,
    ADC_100mV = 1,
    ADC_200mV = 2,
    ADC_500mV = 3,
    ADC_1V = 4,
    ADC_2V = 5,
    ADC_5V = 6,
    ADC_10V = 7,
    ADC_OTHER_CHANNEL = 8 // For interlaced 2x sample speed capture
}

new adc_range_names[8]{} = [
    "50mV", "100mV", "200mV", "500mV", "1V", "2V", "5V", "10V"
];
#pragma unused adc_range_names

const Channel: {
    Ch_A = 0,
    Ch_B = 1,
    Ch_C = 2,
    Ch_D = 3
}

/** DS203 triggering works so that the trigger occurs at sample #150.
 * Therefore the first 150 samples are from before the trigger, and the rest
 * from after it.
 *
 * Unfortunately, for the Trig_Always mode, the presampling does not work
 * and the first 150 samples will be automatically discarded. Therefore
 * you'll only get 3946 output samples in Trig_Always mode.
 */
const TriggerMode: {
    Trig_NegEdge = 0,
    Trig_PosEdge = 1,
    Trig_NegLevel = 2,
    Trig_PosLevel = 3,
    Trig_NegShortPulse = 4, // Negative pulse shorter than T
    Trig_NegLongPulse = 5, // Negative pulse longer than T
    Trig_PosShortPulse = 6,
    Trig_PosLongPulse = 7,
    Trig_Always = 32 // Trigger always, i.e. unconditionally
}

/// Configure settings related to the analog input stage
native config_chA(ADCCoupling: coupling, ADCRange: range, offset = 128);
native config_chB(ADCCoupling: coupling, ADCRange: range, offset = 128);

/// Read back the input config (used by calibration.inc)
native getconfig_chA(&ADCCoupling: coupling, &ADCRange: range, &offset);
native getconfig_chB(&ADCCoupling: coupling, &ADCRange: range, &offset);

/// Set the sample rate of the inputs, in Hz.
/// This tries to achieve a sample rate as close as possible to the given rate.
/// It returns the actual samplerate, which is always less or equal to the
/// requested samplerate.
native wavein_samplerate(frequency);

/// Set the trigger mode
/// mode: Select the trigger event type
/// channel: Select the triggering channel
/// threshold: Set the voltage threshold for triggering
/// pulse_time: Set the time threshold for pulse triggers
/// delay: Delay capture after the trigger event (delayed trigger)
/// interlaced: Capture chA and chB alternately with double samplerate
native wavein_settrigger(TriggerMode: mode, Channel: channel = Ch_A,
                      threshold = 128, pulse_time = 10, delay = 1,
                      bool: interlaced = false);

/// Start the capture, i.e. start looking for trigger
/// If sync is true, the capture is synchronized with the waveout output so
/// that it starts at the same time as the waveout period.
native wavein_start(bool: sync = false);

/// You can monitor the trigger status of the FPGA.
/// When the trigger has occurred, this will return true.
native bool: wavein_istriggered();

/// Read a batch of samples (max. 4096) from selected channels. By passing
/// an empty array for some channel, that channel is not captured.
/// If the trigger has not yet occurred, this function will wait for it.
/// Samples are stored in packed format (8 bits per value), so use {} to index
/// to the arrays. The number of samples read is always a multiple of 4.
native wavein_read(chA{} = {0}, chB{} = {0}, chC{} = {0}, chD{} = {0},
    countA = sizeof chA, countB = sizeof chB, countC = sizeof chC, countD = sizeof chD);

/// Compute the maximum, minimum, sum and sum of squares of the wavein value
/// over several samples for channels A and B.
/// You have to call wavein_start() before calling this function.
stock wavein_aggregate(&sumA = 0, &sumB = 0,
                       &maxA = 0, &maxB = 0,
                       &minA = 0, &minB = 0,
                       &sqsumA = 0, &sqsumB = 0,
                       &Fixed: avgA = FIX(0.0), &Fixed: avgB = FIX(0.0),
                       count = 1000)
{
    new bufA{4}, bufB{4};
    
    sumA = 0; sumB = 0;
    maxA = cellmin; maxB = cellmin;
    minA = cellmax; minB = cellmax;
    sqsumA = 0; sqsumB = 0;
    
    new i = 0;
    while (i < count)
    {
        wavein_read(bufA, bufB);
        
        for (new j = 0; j < 4 && i < count; j++, i++)
        {
            sumA += bufA{j};
            sumB += bufB{j};
            if (bufA{j} > maxA) maxA = bufA{j};
            if (bufB{j} > maxB) maxB = bufB{j};
            if (bufA{j} < minA) minA = bufA{j};
            if (bufB{j} < minB) minB = bufB{j};
            sqsumA += bufA{j} * bufA{j};
            sqsumB += bufB{j} * bufB{j};
        }
    }
    
    avgA = (Fixed:sumA) / (Fixed:count);
    avgB = (Fixed:sumB) / (Fixed:count);
}

/// Automatically select the ADC range based on the input signal level.
/// Keeps other settings the same, so you have to call config_chX() first
/// to set the coupling and offset.
stock wavein_autorange(bool: chA = true, bool: chB = true, low_threshold = 30, high_threshold = 110)
{
    new ADCCoupling: coupleA, ADCRange: rangeA, offsetA;
    getconfig_chA(coupleA, rangeA, offsetA);
    
    new ADCCoupling: coupleB, ADCRange: rangeB, offsetB;
    getconfig_chB(coupleB, rangeB, offsetB);
    
    new bool: changed, iters = 0;
    do
    {
        changed = false;
        iters++;
    
        new minA, minB, maxA, maxB;
        wavein_start();
        wavein_aggregate(.minA = minA, .minB = minB, .maxA = maxA, .maxB = maxB);
        
        new deltaA = max(offsetA - minA, maxA - offsetA);
        new deltaB = max(offsetB - minB, maxB - offsetB);
        
        if (chA)
        {
            if (deltaA > high_threshold && rangeA < ADC_10V)
            { rangeA++; changed = true; }
            else if (deltaA < low_threshold && rangeA > ADC_50mV)
            { rangeA--; changed = true; }
        }
        
        if (chB)
        {
            if (deltaB > high_threshold && rangeB < ADC_10V)
            { rangeB++; changed = true; }
            else if (deltaB < low_threshold && maxB < 200 && rangeB > ADC_50mV)
            { rangeB--; changed = true; }
        }
        
        if (changed)
        {
            if (chA)
                config_chA(coupleA, rangeA, offsetA);
            
            if (chB)
                config_chB(coupleB, rangeB, offsetB);
            
            delay_ms(1);
        }
    } while (changed && iters < 10);
}
