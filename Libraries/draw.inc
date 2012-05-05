/** Display drawing functions
 * These functions draw directly to the LCD screen.
 * Note that they use the DSO Quad screen coordinate system, which has
 * origin (0,0) at the bottom left.
 */

#include <fixed>

/// Converts color specified in red, green and blue values 0-255
/// to the RGB565 format used by the display functions.
#define RGB(%1,%2,%3) (Color:(((%1) >> 3) | (((%2) >> 2) << 5) | (((%3) >> 3) << 11)))
 
const Color:
{
    transparent = -1,
    
    black = RGB(  0,  0,  0),
    white = RGB(255,255,255),
    gray  = RGB(127,127,127),
    red   = RGB(255,  0,  0),
    green = RGB(  0,255,  0),
    blue  = RGB(  0,  0,255),
    yellow= RGB(255,255,  0),
    cyan  = RGB(  0,255,255),
    purple= RGB(255,  0,255),
    
    darkred = RGB(128, 0, 0),
    darkgreen = RGB(0, 128, 0),
    darkblue = RGB(0, 0, 128)
}

const screenwidth = 400;
const screenheight = 240;
const fontwidth = 8;
const fontheight = 14;

/// Draw text. When center is true x,y is the center of the bottom, otherwise
/// the bottom left corner,
native draw_text(const str{}, x, y, Color: fg=white,
                    Color: bg=transparent, bool: center = false);

/// Draw text with automatic word wrapping.
native draw_flowtext(const str{}, x, y, w, h, Color: fg = white, Color: bg = transparent, bool: center = false);

/// Clear the bottom row of screen and draw text there.
stock draw_status(const str{})
{
    new buffer{55} = "";
    strcat(buffer, str);
    strcat(buffer, "                                                      ");
    draw_text(buffer, 0, 0, .bg = black);
}

/// Get the LCD type information, for debugging purposes
native lcd_type();
                    
/// Fill a rectangle with same color
native fill_rectangle(x, y, w, h, Color: color);

/// Fill the whole screen with same color.
stock clear_screen(Color: color=black)
    fill_rectangle(0, 0, 400, 220, color);

/// Set pixel at given location
native putpixel(x, y, Color: color);

/// Read pixel value from screen
native Color: getpixel(x, y);

/// Blend two colors together. Opacity is 0 to 1.
native Color: blend(Color: fg, Color: bg, Fixed: opacity);

/// Write a vertical column of pixels to screen
native putcolumn(x, y, const Color: pixels[], count = sizeof pixels, bool: wait = true);

/// Read a vertical column of pixels
native getcolumn(x, y, Color: pixels[], count = sizeof pixels);

/// Draw antialiased line between two points
native drawline_aa(Fixed: x1, Fixed: y1, Fixed: x2, Fixed: y2, Color: color=white);

/// Draw non-antialiased line between two points.
/// Can optionally have a 1:1 dot pattern.
native drawline(x1, y1, x2, y2, Color: color=white, bool: dots=false);

/// Draw a rectangle outline
native draw_rectangle(x, y, w, h, Color: color=white, bool: dots=false);

/// Adjust x1, w and x2 so that they are all positive and less than max.
/// Helper function for copy_area
static stock limit_range(&x1, &w, &x2, max)
{
    if (x2 < 0)
    {
        x1 -= x2;
        w -= x2;
        x2 = 0;
    }
    
    if (x2 + w > max)
    {
        w = max - x2;
    }
}

/// Copy a screen area (x1,y1,w,h) to another location (x2,y2)
/// (x1,y1) and (x2,y2) are the bottom left corners of the areas.
stock copy_area(x1, y1, w, h, x2, y2)
{
    new Color: tmp[screenheight];
    limit_range(x1, w, x2, screenwidth);
    limit_range(y1, h, y2, screenheight);
    
    // Choose copy direction so that we don't overwrite data that
    // has not been copied yet.
    if (x2 > x1)
    {
        for (new i = w - 1; i >= 0; i--)
        {
            getcolumn(x1 + i, y1, tmp, h);
            putcolumn(x2 + i, y2, tmp, h);
        }
    }
    else
    {
        for (new i = 0; i < w; i++)
        {
            getcolumn(x1 + i, y1, tmp, h);
            putcolumn(x2 + i, y2, tmp, h);
        }
    }
}

/// Draw a small monochrome bitmap image to screen.
/// The bitmap is defined as a constant array, where each entry is a
/// 32 pixels wide row. E.g., this is a sphere:
/// new const sphere[] = [0b00100,
///                       0b01110,
///                       0b00100];
/// The coordinates x,y correspond to bottom left corner of the bitmap.
native draw_bitmap(const bitmap[], x, y, Color: color, count = sizeof bitmap, bool: center = false);

stock render_polyline(const Fixed: xpoints[], const Fixed: ypoints[],
                      x0, y0, Color: column[],
                      Fixed: width = FIX(1.0), Color: color = white,
                      pointcount = sizeof xpoints, height = sizeof column)
{
    new Fixed: xpos = fixed(x0);
    
    new Fixed: olddelta = xpoints[0] - xpos;
    for (new i = 1; i < pointcount; i++)
    {
        new Fixed: newdelta = xpoints[i] - xpos;
        if (newdelta == 0 ||
            (newdelta > 0) != (olddelta > 0) ||
            (newdelta == olddelta && absf(newdelta) < width))
        {
            // The line is drawn by four points, which define an intensity
            // curve:
            //        B------C        Pixel
            //       /        \       intensity   
            //  ____A          D_____
            //
            //    Pixel coordinate
            //
            // The intensity curve is sampled discretely, the slopes at the
            // end implement antialiasing.
            
            new Fixed: x1 = xpoints[i-1], Fixed: x2 = xpoints[i];
            new Fixed: y1 = ypoints[i-1], Fixed: y2 = ypoints[i];
            new Fixed: ratio = FIX(1.0);
            
            if (y1 > y2)
            {
                x1 = xpoints[i]; x2 = xpoints[i-1];
                y1 = ypoints[i]; y2 = ypoints[i-1];
            }
            
            new Fixed: A, Fixed: B, Fixed: C, Fixed: D;
            
            if (x1 == x2)
            {
                // A vertical line
                A = y1 - 1;
                B = y1;
                C = y2;
                D = y2 + 1;
                
                new Fixed: delta = absf(xpos - x1);
                new Fixed: halfwidth = (width - 1) / 2;
                if (delta > halfwidth)
                    ratio = halfwidth + 1 - delta;
            }
            else if (y1 == y2)
            {
                // A horizontal line
                new Fixed: halfwidth = (width - 1) / 2;
                B = y1 - halfwidth;
                C = y1 + halfwidth;
                A = B - 1;
                D = C + 1;
            }
            else
            {
                // Sloped line
                //    | x |/ 
                //    |   D  
                //    |  / / 
                //    | / B  
                //    |/ /|
                //    C / |  
                //   / /  |  
                //  / A   |
                // / /|   |
                
                new Fixed: xd = x2 - x1, Fixed: yd = y2 - y1;
                new Fixed: n = sqrt(xd * xd + yd * yd);
                new Fixed: halfwidth = absf(xd) / n * (width - 1) / 2; 
                A = interpolate(xpos - FIX(0.5), x1, y1, x2, y2) - halfwidth;
                B = interpolate(xpos + FIX(0.5), x1, y1, x2, y2) - halfwidth;
                C = interpolate(xpos - FIX(0.5), x1, y1, x2, y2) + halfwidth;
                D = interpolate(xpos + FIX(0.5), x1, y1, x2, y2) + halfwidth;
                
                if (xd < 0)
                {
                    new Fixed: tmp = A;
                    A = B;
                    B = tmp;
                    
                    tmp = C;
                    C = D;
                    D = tmp;
                }
                
                A -= FIX(1.0);
                D += FIX(1.0);
            }
            
            if (ratio > 0)
            {
                for (new y = fround(A); y <= fround(D); y++)
                {
                    new idx = y - y0;
                    if (idx < 0 || idx >= height) continue;
                    
                    new Color: p = column[idx];
                    
                    if (y >= B && y <= C)
                        p = blend(color, p, ratio);
                    else if (y < B)
                        p = blend(color, p, ratio * (y - A) / (B - A));
                    else
                        p = blend(color, p, ratio * (D - y) / (D - C));
                    
                    column[idx] = p;
                }
            }
        }
        olddelta = newdelta;
    }
}

/// Save a bitmap to a file.
/// You can give custom colors in palette, which will be added to the default palette. If more than 2 colors are given, they overwrite some of the default colors.
native bool: save_bitmap(const filename{}, Color: palette[] = [black], palette_size = sizeof palette);
