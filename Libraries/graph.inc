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

/// This defines a pseudo-struct for configuring the graph axes
#define GraphAxis [
    /// Color of the tick marks and tick labels
    Color: .tickcolor,
    
    /// Coordinate range for the axis.
    Fixed: .min,
    Fixed: .max,
    
    /// Interval between the major grid lines
    Fixed: .major,
    
    /// Number of minor grid lines per each major line.
    /// I.e. major grid 0.5, minor grid 5 => minor grid interval 0.1.
           .minor,
    
    /// Set to true to use a logarithmic scale for the axis.
    /// The meaning of min, max and point coordinates changes: you should set
    /// them to log10 of the actual value. Use log10i() if the values are
    /// larger than 10000.    
    bool:  .log,
    
    /// This is used internally to tell apart the x and y axes. You shouldn't
    /// change its value.
    bool: .vertical
]

new graph_xaxis[GraphAxis] = [_:white, _:FIX(0.0), _:FIX(250.0), _:FIX(50.0), 5, false, false];
new graph_yaxis[GraphAxis] = [_:white, _:FIX(0.0), _:FIX(250.0), _:FIX(50.0), 5, false, true];

/// Y2 axis is disabled when its max == min.
new graph_y2axis[GraphAxis] = [_:yellow, _:FIX(0.0), _:FIX(0.0), _:FIX(50.0), 5, false, true];

stock graph_getaxislength(bool: vertical, &start, &length)
{
    if (vertical)
    {
        start = graph_y;
        length = graph_h;
    }
    else
    {
        start = graph_x;
        length = graph_w;
    }
}

/// Compute pixel coordinate along an axis
stock Fixed: graph_topixels(Fixed: value, const axis[GraphAxis])
{
    new start, length;
    graph_getaxislength(axis[.vertical], start, length);
    
    if (value > axis[.max]) value = axis[.max];
    if (value < axis[.min]) value = axis[.min];
    return start + (value - axis[.min]) * (length / (axis[.max] - axis[.min]));
}

stock Fixed: graph_getx(Fixed: value)
    return graph_topixels(value, graph_xaxis);

stock Fixed: graph_gety(Fixed: value)
    return graph_topixels(value, graph_yaxis);

stock Fixed: graph_gety2(Fixed: value)
    return graph_topixels(value, graph_y2axis);
    
/// Compute graph coordinate along an axis from pixel coordinate.
stock Fixed: graph_frompixels(Fixed: pixels, const axis[GraphAxis])
{
    new start, length;
    graph_getaxislength(axis[.vertical], start, length);
    
    return axis[.min] + (pixels - start) * ((axis[.max] - axis[.min]) / length);
}
    
static stock Fixed: first_major(const axis[GraphAxis])
{
    new Fixed: value = axis[.min];
    new Fixed: multiple = axis[.major];
    if (value % multiple == 0)
        return value;
    
    return value + multiple - value % multiple;
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
        else if (absf(value) < FIX(0.1) && value != 0)
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

/// This is used to speed up the grid drawing.
static Color: y_column[240];

/// Draws the tickmarks and borders for the graph.
/// You probably want to clear the screen before calling this.
stock draw_axes()
{
    memset(_:y_column, _:graph_bg);
    
    // X axis ticks
    for (new Fixed: x = first_major(graph_xaxis); x <= graph_xaxis[.max]; x += graph_xaxis[.major])
    {
        new label{12};
        label = get_ticklabel(x, graph_xaxis[.log]);
        
        new xpix = fround(graph_getx(x));
        xpix -= strlen(label) * fontwidth / 2;
        
        draw_text(label, xpix, graph_y - fontheight, graph_xaxis[.tickcolor]);
    }
    
    // Y axis ticks
    for (new Fixed: y = first_major(graph_yaxis); y <= graph_yaxis[.max]; y += graph_yaxis[.major])
    {
        new label{12};
        label = get_ticklabel(y, graph_yaxis[.log]);
        
        new xpos = graph_x - strlen(label) * fontwidth - 5;
        new ypix = fround(graph_gety(y));
        y_column[ypix] = graph_major_grid_color;
        ypix -= fontheight / 2;

        draw_text(label, xpos, ypix, graph_yaxis[.tickcolor]);
        
        // Draw minor gridlines
        new Fixed: increment = graph_yaxis[.major] / graph_yaxis[.minor];
        for (new i = 0; i < graph_yaxis[.minor]; i++)
        {
            ypix = fround(graph_gety(y + i * increment));
            if (y_column[ypix] == graph_bg)
                y_column[ypix] = graph_minor_grid_color;
        }
    }
    
    // Y2 axis ticks
    if (graph_y2axis[.min] != graph_y2axis[.max])
    {
        for (new Fixed: y = first_major(graph_y2axis); y <= graph_y2axis[.max]; y += graph_y2axis[.major])
        {
            new label{12};
            label = get_ticklabel(y, graph_y2axis[.log]);
            
            new ypix = fround(graph_gety2(y));
            ypix -= fontheight / 2;
            
            draw_text(label, graph_x + graph_w + 5, ypix, graph_y2axis[.tickcolor]);
        }
    }
    
    // Graph border
    draw_rectangle(graph_x - 1, graph_y - 1, graph_w + 1, graph_h + 1, graph_border);
}

/// Get the closest gridline to pos. Returns the graph coordinate.
static stock Fixed: closest_gridline(Fixed: pos, Fixed: increment)
{
    pos += increment / 2;
    return pos - pos % increment;
}

/// Get the closest minor gridline, with special handling for log scale.
static stock Fixed: closest_minorline(Fixed: pos, axis[GraphAxis])
{
    if (!axis[.log])
        return closest_gridline(pos, axis[.major] / axis[.minor]);
    
    new Fixed: prev_major = pos - pos % axis[.major];
    new Fixed: step = axis[.major] / (axis[.minor]);
    new Fixed: delta = exp((pos - prev_major - axis[.major]) * log_of_10);
    new i = fround(delta / step);
    return prev_major + 1 + log10(i * step);
}

/// Render one column of the graph, starting at x0, y0. This is used for
/// redrawing the graph and the contents in one go without flicker.
stock render_graph_column(x0, y0, Color: column[], height = sizeof column)
{
    if (x0 <= graph_x || x0 >= graph_x + graph_w)
        return;
    
    new Fixed: xcoord = graph_frompixels(x0, graph_xaxis);
    if (x0 == fround(graph_getx(closest_gridline(xcoord, graph_xaxis[.major]))))
    {
        // Major X line
        memset(_:column, _:graph_major_grid_color, height);
    }
    else if (x0 == fround(graph_getx(closest_minorline(xcoord, graph_xaxis))))
    {
        // Minor X line
        memcpy(_:column, _:y_column[y0], height);
        for (new i = 0; i < height; i++)
        {
            if (column[i] == graph_bg)
                column[i] = graph_minor_grid_color;
        }
    }
    else
    {
        // Between X lines, draw Y grid
        memcpy(_:column, _:y_column[y0], height);
    }
}

/// Clear the graph area and draw grid, borders and ticks.
stock clear_graph()
{
    draw_axes();
    
    new Color: column[240];
    for (new x = graph_x; x < graph_x + graph_w; x++)
    {
        render_graph_column(x, graph_y, column, graph_h);
        putcolumn(x, graph_y, column, graph_h);
    }
}

/// Draw a line on the graph, continuing from a previous point.
/// You can initialize prevx to fix16_min, so the function knows to automatically write the first x coordinate there.
stock graph_draw(Fixed: x, Fixed: y, &Fixed: prevx, &Fixed: prevy, Color: color = green)
{
    if (prevx != fix16_min)
    {
        drawline_aa(graph_getx(prevx), graph_gety(prevy), graph_getx(x), graph_gety(y), color);
    }
    
    prevx = x;
    prevy = y;
}

stock graph_drawy2(Fixed: x, Fixed: y, &Fixed: prevx, &Fixed: prevy, Color: color = yellow)
{
    if (prevx != fix16_min)
    {
        drawline_aa(graph_getx(prevx), graph_gety2(prevy), graph_getx(x), graph_gety2(y), color);
    }
    
    prevx = x;
    prevy = y;
}