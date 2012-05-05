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
stock print(const str{}, bool:wordwrap = true)
{
    new strpos = 0;
    new line{50};
    
    while (str{strpos} != 0)
    {
        new cutpos = strchr(str, '\n', strpos);
        
        if (cutpos == -1)
            cutpos = strlen(str);
        
        new cols = cons_cols - cons_col;
        if (cutpos - strpos > cols)
        {
            cutpos = strpos + cols;

            if (wordwrap)
            {
                new wrappos = max(
                    strrchr(str, ' ', cutpos),
                    strrchr(str, '-', cutpos) + 1
                );
            
                if (wrappos > strpos || (wrappos == strpos && cons_col != 0))
                    cutpos = wrappos;
            }
        }
        
        if (cutpos != strpos)
        {
            strmid(line, str, strpos, cutpos - strpos);
            
            draw_text(line,
                        cons_col * fontwidth + cons_x,
                        cons_row * fontheight + cons_y,
                        cons_fg, cons_bg);
            cons_col += cutpos - strpos;
            strpos = cutpos;
        }
        
        if (str{strpos} != 0)
        {
            if (cons_row == 0)
                scroll_down(1);
        
            cons_row -= 1;
            cons_col = 0; 
            
            if (str{strpos} == '\n' || str{strpos} == ' ')
                strpos++;
        }
    }
}

/// Convenience wrappers
stock println(const str{})
{
    print(str);
    print("\n");
}

stock printsp(const str{})
{
    print(str);
    print(" ");
}
