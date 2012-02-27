/** A function library for graph drawing. Supports mainly line graphs,
 * with one X axis and 1 or 2 Y axes. Any axis can optionally be made
 * logarithmic.
 */

#include <draw>
#include <string>
#include <fixed>

/// Size and location of the graph. Note that the axis labels etc. will be
/// drawn outside this area, so reserve ~20 pixels for that.
new graph_x = 40, graph_y = 40, graph_w = 320, graph_h = 160;

/// Color of the background
new Color: graph_bg = black;

/// Color of the grids
new Color: graph_major_grid_color = red;
new Color: graph_minor_grid_color = RGB(128,0,0);

/// Color of the border around graph
new Color: graph_border = white;

/// Color of the tick labels
new Color: graph_xcolor = white, Color: graph_ycolor = white;
new Color: graph_y2color = green;

/// Scaling info for the X axis:
/// xmin, xmax: the minimum and maximum value of the X axis
/// xmajor: the step between major gridlines
/// xminor: the divider of xmajor to get minor grid step
new Fixed: graph_xmin = FIX(0.0), Fixed: graph_xmax = FIX(250.0);
new Fixed: graph_xmajor = FIX(50.0), graph_xminor = 5;

/// xlog: use logarithmic X axis
/// The meaning of xmin, xmax and x coordinates changes: you should set them
/// to log10 of the actual value. Use log10i() if the values are larger than
/// 10000.
new bool: graph_xlog = false;

/// Scaling for the left Y axis
new Fixed: graph_ymin = FIX(0.0), Fixed: graph_ymax = FIX(250.0);
new Fixed: graph_ymajor = FIX(50.0), graph_yminor = 5;
new bool: graph_ylog = false;

/// Scaling for the right Y axis (set y2min = y2max to disable)
new Fixed: graph_y2min = FIX(0.0), Fixed: graph_y2max = FIX(0.0);
new Fixed: graph_y2major = FIX(20.0);
new bool: graph_y2log = false;


/// Compute pixel coordinate along an axis
stock Fixed: graph_getpixels(Fixed: value, Fixed: min, Fixed: max, start, length)
{
    if (value > max) value = max;
    if (value < min) value = min;
    return start + (value - min) * (length / (max - min));
}

/// Compute pixel coordinate for x axis
stock Fixed: graph_getx(Fixed: value)
    return graph_getpixels(value, graph_xmin, graph_xmax, graph_x, graph_w);

/// Compute pixel coordinate for the left axis
stock Fixed: graph_gety(Fixed: value)
    return graph_getpixels(value, graph_ymin, graph_ymax, graph_y, graph_h);

/// Compute pixel coordinate for y2 axis
stock Fixed: graph_gety2(Fixed: value)
    return graph_getpixels(value, graph_y2min, graph_y2max, graph_y, graph_h);

static stock Fixed: align(Fixed: value, Fixed: multiple)
{
    if (value >= 0)
    {
        return value - value % multiple;
    }
    else
    {
        return value + value % multiple;
    }
}

stock get_ticklabel(Fixed: value, bool: log)
{
    new result{12};
    new suffix{2} = {0,0};
    
    if (log)
    {
        if (value >= 6)
        {
            value = value - 6;
            suffix{0} = 'M';
        }
        else if (value >= 3)
        {
            value = value - 3;
            suffix{0} = 'k';
        }
        else if (value < -6)
        {
            value = value + 9;
            suffix{0} = 'n';
        }
        else if (value < -3)
        {
            value = value + 6;
            suffix{0} = 'u';
        }
        else if (value < 0)
        {
            value = value + 3;
            suffix{0} = 'm';
        }
        
        result = strf(value, 0);
        result{0} = '1';
        new i;
        for (i = 1; value > 0; i++, value--)
        {
            result{i} = '0';
        }
        result{i} = 0;
    }
    else
    {
        if (absf(value) >= 1000)
        {
            value = value / 1000;
            suffix{0} = 'k';
        }
        else if (absf(value) < 1 && value != 0)
        {
            value = value * 1000;
            suffix{0} = 'm';
        }
        
        if (value < FIX(10.0) && ftrunc(value) * 10 != ftrunc(value * 10))
        {
            // Value is 1.x
            result = strf(value, 1);
        }
        else
        {
            result = strf(value, 0);
        }
    }
    
    strcat(result, suffix);
    return result;
}
    
/// Clear the graph area and draw grid, borders and ticks.
stock clear_graph()
{
    // Reset the draw function "previous value" variables
    graph_draw(fix16_min, fix16_min);
    graph_drawy2(fix16_min, fix16_min);
    
    fill_rectangle(graph_x, graph_y, graph_w, graph_h, graph_bg);
    
    // Draw x axis grid
    for (new Fixed: x = align(graph_xmin, graph_xmajor); x <= graph_xmax; x += graph_xmajor)
    {
        new xpix = fround(graph_getx(x));
        
        if (x >= graph_xmin)
        {
            // Major line
            drawline(xpix, graph_y, xpix, graph_y + graph_h, graph_major_grid_color, true);
            
            // Tick label
            new label{12};
            label = get_ticklabel(x, graph_xlog);
            xpix -= strlen(label) * fontwidth / 2;
            draw_text(label, xpix, graph_y - fontheight, graph_xcolor);
        }
        
        // Minor lines
        new Fixed: step = graph_xmajor / graph_xminor;
        new Fixed: mx;
        for (new i = 1; i < graph_xminor; i++)
        {
            new Fixed: pos = i * step;
            if (graph_xlog)
                mx = x + 1 + log10(pos);
            else
                mx = x + pos;
        
            xpix = fround(graph_getx(mx));
            drawline(xpix, graph_y, xpix, graph_y + graph_h, graph_minor_grid_color, true);
        }
    }
    
    // Draw y2 axis grid
    if (graph_y2min != graph_y2max)
    {
        for (new Fixed: y = align(graph_y2min, graph_y2major); y <= graph_y2max; y += graph_y2major)
        {
            new ypix = fround(graph_gety2(y));
            
            if (y >= graph_y2min)
            {
                // Major line
                drawline(graph_x, ypix, graph_x + graph_w, ypix, graph_y2color, true);
                
                // Tick label
                new label{12};
                label = get_ticklabel(y, graph_y2log);
                ypix -= fontheight / 2;
                draw_text(label, graph_x + graph_w + 3, ypix, graph_y2color);
            }
        }
    }
    
    // Draw y axis grid
    for (new Fixed: y = align(graph_ymin, graph_ymajor); y <= graph_ymax; y += graph_ymajor)
    {
        new ypix = fround(graph_gety(y));
        
        if (y >= graph_ymin)
        {
            // Major line
            drawline(graph_x, ypix, graph_x + graph_w, ypix, graph_major_grid_color, true);
            
            // Tick label
            new label{12};
            label = get_ticklabel(y, graph_ylog);
            new xpos = graph_x - strlen(label) * fontwidth - 3;
            ypix -= fontheight / 2;
            draw_text(label, xpos, ypix, graph_ycolor);
        }
        
        // Minor lines
        new Fixed: step = graph_ymajor / graph_yminor;
        new Fixed: my;
        for (new i = 1; i < graph_yminor && my < graph_ymax; i++)
        {
            new Fixed: pos = i * step;
            if (graph_ylog)
                my = y + 1 + log10(pos);
            else
                my = y + pos;

            ypix = fround(graph_gety(my));
            drawline(graph_x, ypix, graph_x + graph_w, ypix, graph_minor_grid_color, true);
        }
    }
    
    draw_rectangle(graph_x, graph_y, graph_w, graph_h, graph_border);
}

/// Draw a line on the graph.
/// If x2 and y2 are not specified, continue from previous point.
stock graph_draw(Fixed: x1, Fixed: y1, Fixed: x2 = fix16_min, Fixed: y2 = fix16_min, Color: color = Color: cellmin)
{
    static Fixed: graph_prevx = fix16_min, Fixed: graph_prevy = fix16_min;

    if (x2 == fix16_min)
        x2 = graph_prevx;
        
    if (y2 == fix16_min)
        y2 = graph_prevy;
    
    if (color == Color:cellmin)
        color = graph_ycolor;
    
    graph_prevx = x1;
    graph_prevy = y1;
    
    if (x2 != fix16_min)
        drawline_aa(graph_getx(x1), graph_gety(y1), graph_getx(x2), graph_gety(y2), color);
}

/// Draw a line on the graph, using y2 coordinates
stock graph_drawy2(Fixed: x1, Fixed: y1, Fixed: x2 = fix16_min, Fixed: y2 = fix16_min, Color: color = Color:cellmin)
{
    static Fixed: graph_prevx = fix16_min, Fixed: graph_prevy = fix16_min;

    if (x2 == fix16_min)
        x2 = graph_prevx;
        
    if (y2 == fix16_min)
        y2 = graph_prevy;
    
    if (color == Color:cellmin)
        color = graph_y2color;
    
    graph_prevx = x1;
    graph_prevy = y1;

    if (x2 != fix16_min)
        drawline_aa(graph_getx(x1), graph_gety2(y1), graph_getx(x2), graph_gety2(y2), color);
}
