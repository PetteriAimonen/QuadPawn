/** Functions for a scrolling console window */

#include <draw>

/// You can set these variables from your code to control the location of
/// the console.
new cons_x = 0, cons_y = 0; // Bottom left corner of window
new cons_cols = 50, cons_rows = 15; // Size of window
new cons_col = 0, cons_row = 14; // Current position
new Color: cons_fg = white, Color: cons_bg = black;

/// Clear the console area and return cursor to top left corner.
stock clear_console()
{
    fill_rectangle(cons_x, cons_y, cons_cols * fontwidth,
                   cons_rows * fontheight, cons_bg);
    cons_col = 0;
    cons_row = cons_rows - 1;
}

/// Scroll down the console the specified number of lines.
stock scroll_down(lines)
{
    copy_area(cons_x, cons_y, cons_cols * fontwidth,
        (cons_rows - lines) * fontheight, cons_x, cons_y + lines * fontheight);
    fill_rectangle(cons_x, cons_y, cons_cols * fontwidth, lines * fontheight, cons_bg);
    cons_row += lines;
}

/// Print text to console. Linefeeds (\n) switch to next line, as
/// does writing past the edge of the console. When the bottom-most
/// line is full, the console scrolls down.
stock print(const str{})
{
    new strpos = 0;
    new linepos = 0;
    new line{50};
    
    do
    {
        new end = cons_cols - cons_col;
        while (linepos < end && str{strpos} != 0 && str{strpos} != '\n')
            line{linepos++} = str{strpos++};
        
        if (linepos != 0)
        {
            line{linepos} = 0;
            draw_text(line,
                        cons_col * fontwidth + cons_x,
                        cons_row * fontheight + cons_y,
                        cons_fg, cons_bg);
            cons_col += linepos;
        }
        
        if (str{strpos} != 0)
        {
            if (cons_row == 0)
                scroll_down(1);
        
            cons_row -= 1;
            cons_col = 0; 
            linepos = 0;
            
            if (str{strpos} == '\n')
                strpos++;
        }
    } while (str{strpos} != 0);
}

/// Convenience wrapper
stock println(const str{})
{
    print(str);
    print("\n");
}
