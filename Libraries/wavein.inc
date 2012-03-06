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
native wavein_read(chA{}, chB{} = {0}, chC{} = {0}, chD{} = {0},
    countA = sizeof chA, countB = sizeof chB, countC = sizeof chC, countD = sizeof chD);

/// Start a continuous capture (not limited to 4096 samples).
/// You need to provide a storage area for FIFO. The larger the buffer, the
/// more time you have between calls to wavein_realtime_read(). However, you
/// shouldn't directly read from the FIFO without using wavein_realtime_read().
/// Samplerates higher than 100kHz probably don't work, because you can't
/// process the samples fast enough. Returns the actual samplerate.
native wavein_realtime_start(fifo[], samplerate, count = sizeof fifo);

/// Fetch samples from the continuous capture buffer.
/// You must call this function periodically to fetch the new samples.
/// If the buffer overruns, the capture will stop and this function will return
/// false.
native bool: wavein_realtime_read(chA{}, chB{} = {0}, chC{} = {0}, chD{} = {0},
    countA = sizeof chA, countB = sizeof chB, countC = sizeof chC, countD = sizeof chD);

native wavein_realtime_stop();

/// Start a realtime logic level capture. The logic signals will be compressed
/// using run-length-encoding. The fifo is used as a temporary storage and
/// must be atleast 1kB (256 cells). Samplerate is always 500kHz.
//native wavein_logic_start(fifo[], count = sizeof fifo);

/// Fetch one event from the realtime logic signal buffer.
/// microseconds: time passed since last event
/// levels: the new levels of the channels, in the 4 lowest bits.
/// If the buffer overruns, the capture will stop and this function will return
/// false.
//native bool: wavein_logic_read(&microseconds, &levels);

//native wavein_logic_stop();
