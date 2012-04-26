#include <graph>
#include <fourier>
#include <console>
#include <config>
#include <calibration>

new inbuf{3000}, refbuf{3000};
new outbuf[512];

new minfreq_idx, minfreq;
new minfreq_labels[]{} = ["10Hz", "100Hz", "1kHz", "10kHz", "100kHz"];
new minfreqs[] = [10, 100, 1000, 10000, 100000];

new maxfreq_idx, maxfreq;
new maxfreq_labels[]{} = ["20kHz", "200kHz", "2MHz", "20MHz"];
new maxfreqs[] = [20000, 200000, 2000000, 20000000];

new waveform_idx;
new waveforms[]{} = ["Square wave", "Sinewave"];

new bool: always_clear;

new bool: use_reference;

new steps_labels[]{} = ["50", "100", "200", "500"];
new steps_values[] = [50, 100, 200, 500];
new steps_idx;

new freqs[550];
new Fixed: amplitudes[550];
new Fixed: phases[550];
new results_count = 0;

new cursor_idx = 0;
new cursor_xpos = 0;

configure(bool: load_only = false)
{
    start_config("freqresp.ini", load_only);
    while (do_config())
    {
        config_multichoice("minfreq", "Minimum frequency", minfreq_idx, minfreq_labels, 1);
        config_multichoice("maxfreq", "Maximum frequency", maxfreq_idx, maxfreq_labels, 1);
        
        config_multichoice("waveform", "Waveform", waveform_idx, waveforms, 1, .count = (maxfreq_idx > 1 ? 1 : 2),
            .help = "The measurement can be done using either square wave or sine wave signal. Square wave can go to higher frequencies, but it may disturb e.g. audio equipment more than sine wave."
        );
        
        config_boolean("clear", "Always clear screen", always_clear, true,
            .help = "If 'Always clear screen' is enabled, the screen will be automatically cleared before each measurement. Otherwise you can clear the screen by holding down button 1 for 1 second when starting the measurement."
        );
        
        config_multichoice("steps", "Number of steps", steps_idx, steps_labels, 1,
            .help = "Total number of measurements to take. The steps will be distributed logarithmically between minimum and maximum frequency, so that each decade has equal number of measurement points."
        );
        
        config_boolean("reference", "Use Ch B as reference", use_reference, false,
            .help = "You can use Channel B as reference signal by connecting it directly together with Wave Out. Input signal goes to Channel A as always. Using a reference input reduces the measurement errors caused by DAC and timing inaccuracies.");
    }
    
    minfreq = minfreqs[minfreq_idx];
    maxfreq = maxfreqs[maxfreq_idx];
    
    graph_xaxis[.min] = log10i(minfreq);
    graph_xaxis[.max] = log10i(maxfreq);

    clear_screen();
    clear();
}

clear()
{
    clear_graph();
    cursor_idx = cursor_xpos = 0;
}

@button1()
{
    draw_menubar("", "", "", "Cancel");
    config_chA(ADC_AC, ADC_500mV);
    wavein_settrigger(Trig_Always);
    waveout_voltage(FIX(1.4));
    
    if (always_clear)
        clear();
    
    // Holding down button 1 clears the screen
    new time;
    while (held_keys(BUTTON1, time))
    {
        if (time > 1000)
            clear();
    }
    
    new Fixed: prevx = fix16_min, Fixed: prevy;
    new Fixed: prevx2 = fix16_min, Fixed: prevy2;
    new i = 0;
    new Fixed: step = exp(log10i(maxfreq/minfreq) / steps_values[steps_idx] * log_of_10);
    for (new freq = minfreq; freq < maxfreq; freq = _:(Fixed:freq * step))
    {
        new outf = freq;
        new Fixed: ref_real, Fixed: ref_imag; // Reference response
        if (waveform_idx == 0)
        {
            outf = waveout_digital(freq);
            ref_real = FIX(0.0);
            ref_imag = FIX(0.81);
        }
        else
        {
            waveout_sinewave(outbuf, freq);
            ref_real = FIX(0.64);
            ref_imag = FIX(0.0);
        }
        
        new inf = wavein_samplerate(outf * 1000);
        
        delay_ms(50);
        
        wavein_autorange();
        
        new ADCCoupling: coupling, ADCRange: range, offset;
        
        if (outf > 1000000 && use_reference)
        {
            // Better use the same range for both, because the
            // range affects frequency response at high freq.
            getconfig_chB(coupling, range, offset);
            config_chA(coupling, range);
        }
        
        wavein_start(true);
        wavein_read(inbuf, refbuf);
        
        new Fixed: real, Fixed: imag;
        dft(inbuf, real, imag, (Fixed:inf)/(Fixed:outf), 3000);
        
        if (use_reference)
        {
            getconfig_chB(coupling, range, offset);
            dft(refbuf, ref_real, ref_imag, (Fixed:inf)/(Fixed:outf), 3000);
            ref_real = ref_real / calibration_scales[range][Ch_B];
            ref_imag = ref_imag / calibration_scales[range][Ch_B];
        }
        
        cdiv(real, imag, ref_real, ref_imag);
        
        new Fixed: magnitude = cabs(real, imag);
        
        getconfig_chA(coupling, range, offset);
        magnitude = magnitude / calibration_scales[range][Ch_A];
        magnitude = 20 * log10(magnitude);
        
        graph_draw(log10i(outf), magnitude, prevx, prevy, white);
        
        new Fixed: phase = rad2deg(carg(real, imag));
        if (absf(prevy2 - phase) > 150)
            prevx2 = prevy2 = fix16_min;
        
        if (magnitude < -40)
        {
            prevx2 = prevy2 = fix16_min;
        }
        else
        {
            graph_drawy2(log10i(outf), phase, prevx2, prevy2);
        }
        
        draw_status(strjoin(str(outf), " Hz:  ",
                            strf(magnitude, 2), " dB  ",
                            strf(phase, 1), " deg"));
        
        if (get_keys(BUTTON4))
            break;
        
        freqs[i] = outf;
        amplitudes[i] = magnitude;
        phases[i] = phase;
        i++;
    }
    results_count = i;
    
    get_keys(ANY_KEY);
}

@button2()
{
    // File saving
    draw_menubar("BMP", "CSV", "", "");
    while (!peek_keys(BUTTON1 | BUTTON2)) {}
    
    if (get_keys(BUTTON1))
    {
        new filename{14} = "FREQ%03d.BMP";
        select_filename(filename);
        
        draw_status(strjoin("Saving ", filename));
        if (save_bitmap(filename))
            draw_status(strjoin("Saved ", filename));
        else
            draw_status("Bitmap save failed!");
    }
    else if (get_keys(BUTTON2))
    {
        new filename{14} = "FREQ%03d.CSV";
        select_filename(filename);
        
        draw_status(strjoin("Saving ", filename));
        
        new File: f = f_open(filename, FA_WRITE | FA_CREATE_NEW);
        f_write(f, "# Freq    dB   Phase(deg)\r\n");
        for (new i = 0; i < results_count; i++)
        {
            f_write(f, strjoin(str(freqs[i]), "\t", strf(amplitudes[i]),
                              "\t", strf(phases[i]), "\r\n"));
        }
        
        if (f_close(f))
            draw_status(strjoin("Saved ", filename));
        else
            draw_status("CSV save failed!");
    }
}

@button3()
{
    configure();
}

drawcursor(x)
{
    new Color: column[240];
    getcolumn(x, graph_y, column, graph_h);
    for (new i = 0; i < graph_h; i++)
        column[i] ^= Color:0xFFFF;
    putcolumn(x, graph_y, column, graph_h);
}

@scroll2(delta)
{
    if (results_count == 0)
        return;
    
    if (delta < 0)
        cursor_idx -= scroller_speed();
    else
        cursor_idx += scroller_speed();
    
    cursor_idx = clamp(cursor_idx, 0, results_count - 1);
    
    if (cursor_xpos != 0)
        drawcursor(cursor_xpos);
    
    cursor_xpos = fround(graph_getx(log10i(freqs[cursor_idx])));
    drawcursor(cursor_xpos);
    
    draw_status(strjoin(str(freqs[cursor_idx]), " Hz:  ",
                        strf(amplitudes[cursor_idx], 2), " dB  ",
                        strf(phases[cursor_idx], 1), " deg "));
}

bool: @idle()
{
    draw_menubar("Measure", "Save", "Config", "Quit");
    get_keys(ANY_KEY);
    wait_keys(ANY_KEY, 120000);
    return !get_keys(BUTTON4);
}

main()
{
    load_calibration();
    
    graph_yaxis[.min] = FIX(-70.0);
    graph_yaxis[.max] = FIX(10.0);
    graph_yaxis[.major] = FIX(10.0);
    
    graph_y2axis[.min] = FIX(-180.0);
    graph_y2axis[.max] = FIX(180.0);
    graph_y2axis[.major] = FIX(90.0);
    
    graph_xaxis[.log] = true;
    graph_xaxis[.major] = FIX(1.0);
    graph_xaxis[.minor] = 10;
    
    configure(true);
    delay_ms(1000);
}

/* Program info */
new const program_icon[] = [
    0b11000000000000000000000000000000,
    0b11000000000000000000000000000000,
    0b11000000000000000000000000000000,
    0b11000000000000000000000000000000,
    0b11000000000001111111100000000000,
    0b11000000000110000000011000000000,
    0b11000000001000000000000100000000,
    0b11000000010000000011000010000000,
    0b11000000100000000100100001000000,
    0b11000001000000000100000000100000,
    0b11000010000000001000000000010000,
    0b11000100000000011110000000001000,
    0b11001000000000001000000000000100,
    0b11010000000000001000000000000010,
    0b11100000000000001000000000000001,
    0b11000000000001010000000000000000,
    0b11000000000000100000000000000000,
    0b11000000000000000000000000000000,
    0b11111111111111111111111111111111,
    0b11111111111111111111111111111111,
];
new const program_name{} = "Frequency Response";
#include <metadata>
