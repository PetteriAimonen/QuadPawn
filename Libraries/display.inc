/** Display drawing functions
 * These functions draw directly to the LCD screen.
 * Note that they use the DSO Quad screen coordinate system, which has
 * origin (0,0) at the bottom left.
 */

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
    purple= RGB(255,  0,255)
}

const screenwidth = 400;
const screenheight = 240;
const fontwidth = 8;
const fontheight = 14;

/// Draw text, where x,y is the bottom left corner.
native display_text(const str[], x, y, Color: foreground=white,
                    Color: background=transparent);

/// Get the LCD type information, for debugging purposes
native lcd_type();
                    
/// Fill a rectangle with same color
native fill_rectangle(x, y, w, h, Color: color);

/// Fill the whole screen with same color.
stock clear_screen(Color: color=black)
    fill_rectangle(0, 0, 400, 240, color);

/// Set pixel at given location
native putpixel(x, y, Color: color);

/// Read pixel value from screen
native Color: getpixel(x, y);

/// Write a vertical column of pixels to screen
native putcolumn(x, y, const Color: pixels[], count = sizeof pixels);

/// Read a vertical column of pixels
native getcolumn(x, y, Color: pixels[], count = sizeof pixels);

/// Draw antialiased line between two points
native drawline_aa(Fixed: x1, Fixed: y1, Fixed: x2, Fixed: y2, Color: color=white);

/// Draw non-antialiased line between two points.
/// Can optionally have a 1:1 dot pattern.
native drawline(x1, y1, x2, y2, Color: color=white, bool: dots=false);

/// Draw a rectangle outline
stock drawrectangle(x, y, w, h, Color: color=white, bool: dots=false)
{
    drawline(x,     y,     x + w, y,     color, dots);
    drawline(x + w, y,     x + w, y + h, color, dots);
    drawline(x + w, y + h, x,     y + h, color, dots);
    drawline(x    , y + h, x,     y,     color, dots);
}

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
stock draw_bitmap(const bitmap[], x, y, Color: color, count = sizeof bitmap)
{
    // Find out the width of the bitmap and align it properly
    new bits = 0;
    for (new i = 0; i < count; i++)
        bits |= bitmap[i];
    
    new width = 0;
    while (bits)
    {
        bits >>>= 1;
        width++;
    }
    
    // Draw the bitmap
    new mask = 1;
    for (new i = width - 1; i >= 0; i--, mask <<= 1)
    {
        for (new j = 0; j < count; j++)
        {
            if (bitmap[j] & mask)
            {
                putpixel(x + i, y + count - j, color);
            }
        }
    }
}
