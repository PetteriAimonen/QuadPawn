#include <console>
#include <graph>
#include <calibration>

pause()
{
    draw_menubar("Continue");
    while (!get_keys(BUTTON1)) {}
    draw_menubar("");
}

/// Check that the connections have been made properly.
check_connections(Fixed: expected, const helpmsg{}, Fixed: threshold = FIX(30.0))
{
    for(;;)
    {
        new Fixed: chA, Fixed: chB;
        wavein_start();
        wavein_aggregate(.avgA = chA, .avgB = chB);
        
        new bool: chAok = (absf(chA - expected) < threshold);
        new bool: chBok = (absf(chB - expected) < threshold);
        
        if (chAok && chBok) return;
        
        new msg{200} = "The measured values seem a bit strange:\n";
        strcat(msg, "ChA: "); strcat(msg, strf(chA, 0));
        strcat(msg, "\nChB: "); strcat(msg, strf(chB, 0));
        strcat(msg, "\nExpected: "); strcat(msg, strf(expected, 0));
        strcat(msg, "\n\n");
        strcat(msg, helpmsg);
        
        new choice = show_dialog(msg, "Retry", "Ignore", "Exit");
        if (choice == 1) return;
        if (choice == 2) exit(0);
    }
}

calibrate_offsets()
{
    graph_xaxis[.max] = FIX(256.0);
    graph_xaxis[.major] = FIX(50.0);
    graph_yaxis[.max] = FIX(256.0);
    graph_yaxis[.major] = FIX(50.0);
    
    clear_screen();
    clear_graph();
    clear_console();
    println("Calibrating the zero-voltage offsets. \
            Connect channels A and B to ground.");
    pause();
    
    // Verify that the channels are properly connected
    waveout_voltage(FIX(1.5));
    config_chA(ADC_DC, ADC_1V);
    config_chB(ADC_DC, ADC_1V);
    delay_ms(100);
    check_connections(FIX(128.0), "Make sure that both channels are connected to the ground.");
    
    clear_screen();
    clear_graph();
    clear_console();
    
    for (new range = 0; range < 8; range++)
    {
        print("Range: "); println(adc_range_names[range]);
        
        new Fixed: prevxA = fix16_min, Fixed: prevyA;
        new Fixed: prevxB = fix16_min, Fixed: prevyB;
        for (new offset = 0; offset <= 256; offset += 128)
        {
            config_chA(ADC_DC, ADCRange:range, .offset = offset);
            config_chB(ADC_DC, ADCRange:range, .offset = offset);
            delay_ms(500);
            
            new Fixed: chA, Fixed: chB;
            wavein_start();
            wavein_aggregate(.avgA = chA, .avgB = chB);
        
            if (offset == 0) clear_graph();
            
            graph_draw(fixed(offset), chA, prevxA, prevyA, green);
            graph_draw(fixed(offset), chB, prevxB, prevyB, yellow);
            
            calibration_offsets[range][0 + offset/128] = chA;
            calibration_offsets[range][3 + offset/128] = chB;
        }
    }
    
    delay_ms(1000);
}

calibrate_scales()
{
    graph_xaxis[.max] = FIX(8.0);
    graph_xaxis[.major] = FIX(1.0);
    graph_yaxis[.max] = FIX(2.0);
    graph_yaxis[.major] = FIX(0.5);
    
    clear_screen();
    clear_graph();
    clear_console();
    println("Calibrating the input scales. \
            Connect channels A and B to Wave Out.");
    pause();
    
    // Verify that the channels are properly connected
    waveout_voltage(FIX(2.0));
    config_chA(ADC_DC, ADC_1V);
    config_chB(ADC_DC, ADC_1V);
    delay_ms(100);
    
    check_connections(FIX(178.0), "Make sure that both channels are connected to waveout.");
    
    clear_screen();
    clear_graph();
    clear_console();
    
    new const Fixed: guess_scales[8] = [
        FIX(500.0), FIX(250.0), FIX(125.0), FIX(50.0),
        FIX(25.0), FIX(12.5), FIX(5.0), FIX(2.5)
    ];
    
    new Fixed: prevxA = fix16_min, Fixed: prevyA;
    new Fixed: prevxB = fix16_min, Fixed: prevyB;
    for (new range = 0; range < 8; range++)
    {
        print("Range: "); println(adc_range_names[range]);
        
        config_chA(ADC_DC, ADCRange:range, .offset = 128);
        config_chB(ADC_DC, ADCRange:range, .offset = 128);
        
        new Fixed: volts = 100 / guess_scales[range];
        if (volts > FIX(2.0)) volts = FIX(2.0);
        
        waveout_voltage(volts);
        delay_ms(500);
            
        new Fixed: chA, Fixed: chB;
        wavein_start();
        wavein_aggregate(.avgA = chA, .avgB = chB);
        
        new Fixed: scaleA = (chA - calibration_offsets[range][1]) / volts;
        new Fixed: scaleB = (chB - calibration_offsets[range][4]) / volts;
        
        calibration_scales[range][0] = scaleA;
        calibration_scales[range][1] = scaleB;
        
        graph_draw(fixed(range), scaleA / guess_scales[range], prevxA, prevyA, green);
        graph_draw(fixed(range), scaleB / guess_scales[range], prevxB, prevyB, yellow);
    }
    
    delay_ms(1000);
}

main()
{
    wavein_settrigger(Trig_Always);

    // Position the console in left half of the screen and the graph in the
    // right half.
    cons_cols = 22;
    
    graph_x = 220;
    graph_w = 160;
    
    calibrate_offsets();
    calibrate_scales();
    
    clear_screen();
    clear_console();
    
    if (show_question("Calibration is done. Do you want to save it?"))
    {
        if (save_calibration())
            print("Calibration saved.");
        else
            print("Calibration saving failed.");
    }
}

new const program_icon[] = [
    0b00000000000000011110000000000000,
    0b00000000000000011111000000000000,
    0b00000000000000011111000000000000,
    0b00000000000000001111000000000000,
    0b00000000000000000111000000000000,
    0b00000000000000000111000000000000,
    0b00000000000000000111000000000000,
    0b00000000000000000011100000000000,
    0b00000000000000000011100000000000,
    0b00000000000000000011100000000000,
    0b00000000000000000001110000000000,
    0b00000000000000000001110000000000,
    0b00000000000000000001110000100000,
    0b00000000000000000000111011110000,
    0b00000000000000000000111100010000,
    0b00000000000000001111000000001000,
    0b00000000000000011000000001001000,
    0b00000000000000010000000001001000,
    0b00000000000000010001001001000100,
    0b00000000000000001001001000100100,
    0b00000000000000001001000100100100,
    0b00000000000000001000100100100010,
    0b00000000000000000100100100010010,
    0b00000000000000000100100010010010,
    0b00000000000000000100010010010001,
    0b00000000000000000010010010000001,
    0b00000000000000000010010001000011,
    0b00000000000000000010000000000110,
    0b00000000000000000001000000011000,
    0b00000000000000000001100111100000,
    0b00000000000000000000111000000000,
    0b00000000000000000000000000000000,
];

new const program_name{} = "Calibration";
#include <metadata>
