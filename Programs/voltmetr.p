/** This is a simple voltage meter application, implemented in PAWN for the
 * DSO Quad portable oscilloscope. It is intended as a demo for the PAWN
 * functionality and usage. Another purpose is to test the calibration
 * accuracy.
 * 
 * The application measures voltage from channels A and B, and displays them
 * on screen. It also keeps track of the maximum and minimum voltage seen in
 * each channel.
 */

// Most common modules are listed in default.inc, and therefore they are
// included automatically. Some such modules are buttons, dialog, draw,
// file, fixed, menu, string, wavein and waveout. However, the calibration
// module takes some RAM space and therefore it is included only on demand.
// You can find the module source code and documentation comments in the
// include folder.
#include <calibration>
#include <console>

// All PAWN programs start with the main function. Optionally, you can also
// use an event-driven approach where the program keeps running after main()
// exits. However, that is not needed for this simple application.
main()
{
    // Having labels for button actions at the top of the screen greatly
    // improves the usability. It does take 20 pixels of space, though.
    clear_screen();
    draw_menubar("Reset", "", "", "Quit");
    
    // Let's then draw the constant texts - these stay in the same place
    // for the duration of the whole program.
    draw_text("DSO Quad Volt Meter", 200, 190, .center = true);
    draw_text("Channel A", 100, 160, .fg = cyan, .center = true);
    draw_text("Channel B", 280, 160, .fg = yellow, .center = true);
    
    // Load calibration.ini. If it is not found, the application will use
    // default calibration, which is often a bit inaccurate.
    load_calibration();
    
    // Set initial configuration for the channels. Wavein_autorange() will
    // adjust the range, but these lines sets the inputs to DC mode and
    // default offset of 128. The channel offset means the ADC value for
    // 0 voltage. I.e. if you set an offset of 0 here, you couldn't measure 
    // negative voltages.
    config_chA(ADC_DC, ADC_2V);
    config_chB(ADC_DC, ADC_2V);
    
    // These configure the ADC timebase and triggering. For a voltage meter,
    // we want continous trigger, i.e. don't wait for any edge. The samplerate
    // is set to 50kHz, so that when we average 1000 samples any 50Hz noise
    // goes away. USA residents might want to use 60kHz instead.
    wavein_settrigger(Trig_Always);
    wavein_samplerate(50000);
    
    // Initialize the variables used to store minimum and maximum values.
    // When we initialize them this way (min with fix16_max, max with
    // fix16_min), the first loop iteration will always overwrite them with
    // the measured value.
    // The variables we use there have the tag 'Fixed'. This means they are
    // a fixed point value, range -32768.00000 to +32767.99998.
    new Fixed: minA = fix16_max, Fixed: maxA = fix16_min;
    new Fixed: minB = fix16_max, Fixed: maxB = fix16_min;
    
    // In the menubar, we said that Button4 would be "Quit". Here we implement
    // that functionality by checking if BUTTON4 has been pressed and stopping
    // the loop if it has. Each time a key is pressed, an interrupt routine
    // sets a flag in a global variable. The flag stays set until someone
    // calls get_keys() with the given button as the parameter. Calling
    // get_keys(ANY_KEY) clears all the key flags.
    while (!get_keys(BUTTON4))
    {
        // The autorange functionality measures the minimum and maximum values
        // from the channels and adjusts the range so that precision is good
        // but values don't exceed the range.
        wavein_autorange();
        
        // This starts the capture, i.e. begins capturing the 4096 samples
        // that fit in the FPGA sample buffer. They will wait in the buffer
        // until you read them out.
        wavein_start();
        
        // Wavein_aggregate reads previously captured samples and calculates
        // the average (and other statistics) about them. It is merely a
        // convenience function, you could also call wavein_read() directly
        // and then calculate the average yourself.
        new Fixed: avgA, Fixed: avgB;
        wavein_aggregate(.avgA = avgA, .avgB = avgB, .count = 1000);
        
        // get_voltage() is the interface to the calibration functionality.
        // It checks the current configuration for a channel (in this case
        // the configuration was set by autorange), and looks up the
        // calibration constants for that configuration. It returns the value
        // converted to volts.
        avgA = get_voltage(Ch_A, avgA);
        avgB = get_voltage(Ch_B, avgB);
        
        draw_text(str(_:avgA), 0, 0, .bg = black);
        
        // Here we update the maximum and minimum values. Note that we update
        // them from the computed average. The averaging process reduces
        // noise, but may cause us to miss some short (less than 20ms)
        // pulses.
        if (avgA > maxA) maxA = avgA;
        if (avgA < minA) minA = avgA;
        if (avgB > maxB) maxB = avgB;
        if (avgB < minB) minB = avgB;
        
        // The text drawing is similar for both channels, so it is separated
        // in a subroutine we can call twice. The parameters simply tell
        // the X coordinate at which to draw, and the color to use.
        print_voltages(100, cyan, avgA, minA, maxA);
        print_voltages(280, yellow, avgB, minB, maxB);
        
        // Finally, Button1 allows us to reset the maximum and minimum values.
        if (get_keys(BUTTON1))
        {
            minA = minB = fix16_max;
            maxA = maxB = fix16_min;
        }
    }
}

/// Figure out a suitable number of decimal digits to use in a value.
/// For values less than 5 volts, we use 3 digits, and for larger values 2
/// digits. For large voltages, the measurement is less accurate and therefore
/// the 3rd decimal would be just random noise so it is better to hide it.
digits(Fixed: value)
{
    return (absf(value) > FIX(5.0)) ? 2 : 3;
}

/// This is the subroutine that prints the voltages on screen. Instead of
/// clearing the screen in between, it just draws the text over at the same
/// position. That avoids some flicker that would be caused by clearing the
/// screen first.
print_voltages(xpos, Color: fg, Fixed: now, Fixed: minval, Fixed: maxval)
{
    // Memory in PAWN must be allocated statically, there is no malloc().
    // This has some implications for string processing, as we must reserve
    // large enough buffer for the string we want to create. Here the voltages
    // are strings like "-3.001 V", so 9 characters should be enough. The last
    // array entry in a string is always a 0 terminator, just like in C.
    //
    // Another point is the use of {}-array for strings and other 8-bit
    // values. This saves some memory compared to a []-array, which has 32-bit
    // entries. All other variables in PAWN are always 32-bit.
    new buf{10};
    
    // Format the current voltage to string. Strf() is a function that
    // converts a fixed point value into string. The second parameter gives
    // the number of digits after the decimal point.
    buf = strf(now, digits(now));
    
    // Append the unit specifier to the string.
    strcat(buf, " V");
    
    // We pad the string from left by spaces, so that it is exactly 8 bytes
    // long. This aligns the values nicely on the screen.
    strpad(buf, 8);
    
    // Now just draw the generated string to screen. By default draw_text()
    // uses a transparent background, so we explicitly specify a black
    // background in order to overwrite the old text.
    draw_text(buf, xpos, 140, .fg = fg, .bg = black);
    
    // Draw a label for the minimum value. This is just constant text that
    // we draw to the left of the actual value. Fontwidth is a constant from
    // draw.inc, which has the width of single character in pixels.
    draw_text("Min: ", xpos - fontwidth * 5, 120, .fg = fg, .bg = black);
    
    // Now just draw the minimum voltage to screen.
    buf = strf(minval, digits(minval));
    strcat(buf, " V");
    strpad(buf, 8);
    draw_text(buf, xpos, 120, .fg = fg, .bg = black);
    
    // Same thing for maximum voltage
    draw_text("Max: ", xpos - fontwidth * 5, 100, .fg = fg, .bg = black);
    buf = strf(maxval, digits(maxval));
    strcat(buf, " V");
    strpad(buf, 8);
    draw_text(buf, xpos, 100, .fg = fg, .bg = black);
}

/// The program list has support for displaying an icon. The icon is specified
/// here as a constant array, and will be compiled into the program. Maximum
/// size is 32 by 32 pixels, monochrome. It is quite easy to edit with a text
/// editor. Another way is to save to .xpm format from The Gimp, which is a
/// similar format and needs just a bit of search & replace to convert.
///
/// The program icon is completely optional, if you leave it out a default
/// icon will be used.
new const program_icon[] = [
    0b0011111111111111111111111111100,
    0b0111111110000001000001111111110,
    0b1111110010000001000001001111111,
    0b1111100001000000000010000011111,
    0b1111100000000000000000000011111,
    0b1110010000000000000000000100111,
    0b1110000000000100010000000000111,
    0b1100000000000100010000000100011,
    0b1110000000000100010000001000111,
    0b1101000000000010100000010001011,
    0b1000000000000010100000100000001,
    0b1000000000000001000001000000001,
    0b1000000000000000000010000000001,
    0b1000000000000000000100000000001,
    0b1000000000000000001000000000001,
    0b1000000000000000010000000000001,
    0b1000000000000000100000000000001,
    0b1100000000000011100000000000011,
    0b1110000000000111110000000000111,
    0b1111111111111111111111111111111,
    0b1111111111111111111111111111111,
    0b1111111111111111111111111111111,
    0b0111111111111111111111111111110,
    0b0011111111111111111111111111100,
];

/// By default, the name of the program is the name of the file, voltmetr.amx.
/// Here we can specify a nicer name, that can be longer and contain spaces.
new const program_name{} = "Volt meter";

/// The metadata module marks the program icon and name for inclusion in the
/// binary. You must specify the icon and/or name first, and then include
/// the module.
#include <metadata>
