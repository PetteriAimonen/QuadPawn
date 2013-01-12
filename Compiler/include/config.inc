/** Configuration dialog & config file library.
 * 
 * This uses an internal state machine to control the loading, editing and
 * saving configuration files. It can be extended with different kinds of
 * on-screen controls for options.
 * 
 * The application code calling this is formulated as a loop:
 * 
 * new numfoos, numbars;
 * start_config("myconf.ini");
 * while (do_config())
 * {
 *     config_integer("numfoos", "Number of foos", numfoos, 0, 0, 10);
 *     config_integer("numbars", "Number of bars", numbars, 0, -100, 100);
 * }
 */

#include <file>
#include <inifile>
#include <buttons>
#include <fixed>
#include <draw>
#include <core>
#include <menu>

const ConfigState: {
    /// The initial configuration state when config is read from a file.
    config_load = 1,
    
    /// Show & edit stage: each control draws itself on the screen and if
    /// it is selected, also responds to keypresses.
    config_edit = 2,
    
    /// Show help for an option
    config_help = 3,
    
    /// Saving state, just before exiting the dialog.
    config_save = 4,
    
    /// Exit the dialog
    config_exit = 5
}

// Constants for the graphical layout
static const label_column = 5;
static const value_column = 250;
static const Color:textcolor = white;
static const Color:bgcolor = black;
static const Color:hilight = blue;
static const rowheight = fontheight;

new ConfigState: config_state;
static ConfigState: next_state;

/// Currently open configuration file. Closed when do_config returns false.
static File: config_file;

/// Index of the control that is currently being drawn. Restarts from zero
/// when do_config() is called.
static current_index;

/// Index of the currently selected control
static selected_index = 0;

static old_selected_index = -1;

/// Does the selected control have help message?
static bool: has_help;

/// Y-coordinate for the current control
static current_y;

/// Handle help message for a control
static do_help(const help{})
{
    if (current_index == selected_index && help{0} != 0)
    {
        if (config_state == config_help)
        {
            show_msgbox(help);
        }
        else
        {
            has_help = true;
        }
    }
}

/// Proceed to next control. Call this at the end of editable control drawing,
/// but not for headers or other passive elements.
static next_control(height = 0)
{
    current_index++;
    current_y -= height;
}

/// Is the current control the selected one? Selected controls should read
/// keyboard signals.
static bool:selected() {return current_index == selected_index;}

/// Draw a selection hilight for one row, if appropiate
static drawbg() {
    fill_rectangle(0, current_y, 400, rowheight,
        selected() ? hilight : bgcolor
    );
}

stock start_config(const filename{}, bool: load_only)
{
    config_file = f_open(filename, FA_READ | FA_WRITE | FA_OPEN_ALWAYS);
    next_state = load_only ? config_load : config_edit;
    selected_index = 0;
    old_selected_index = -1;
    
    clear_screen(bgcolor);
}

stock bool:do_config()
{
    static max_index;
    
    if ((selected_index != old_selected_index && config_state == config_edit)
        || config_state == config_help)
    {
        if (has_help || config_state == config_help)
            draw_menubar("OK", "Cancel", "", "Help");
        else
            draw_menubar("OK", "Cancel", "", "");
        old_selected_index = selected_index;
    }
    
    if (config_state == config_edit && next_state == config_edit)
    {
        // Fill the rest of the screen with black to clear any leftovers
        fill_rectangle(0, 0, 400, current_y + rowheight, bgcolor);
        
        max_index = current_index - 1;
        get_keys(ANY_KEY);
        while (!peek_keys(ANY_KEY)) {}
    }
        
    config_state = next_state;
    
    if (config_state == config_load)
    {
        next_state = config_exit;
    }
    else if (config_state == config_edit)
    {
        if (get_keys(BUTTON1))
        {
            config_state = config_save;
            next_state = config_exit;
        }
        else if (get_keys(BUTTON2))
        {
            config_state = config_load;
            next_state = config_exit;
        }
        else if (get_keys(BUTTON4))
        {
            config_state = config_help;
            next_state = config_edit;
        }
        else if (get_keys(SCROLL2_LEFT))
        {
                selected_index--;
        }
        else if (get_keys(SCROLL2_RIGHT))
        {
                selected_index++;
        }
        
        selected_index = clamp(selected_index, 0, max_index);
    }
    else if (config_state == config_exit)
    {
        clear_screen();
        if (!f_close(config_file))
        {
            new buf{50} = "Failed to save the configuration file: ";
            strcat(buf, str(f_error(config_file)));
            show_msgbox(buf);
        }
        config_file = File:0;
        return false;
    }
    
    if (config_state == config_load)
    {
        f_lseek(config_file, 0);
    }
    else if (config_state == config_edit || config_state == config_help)
    {
        current_index = 0;
        current_y = 200;
    }
    else if (config_state == config_save)
    {
        f_lseek(config_file, 0);
        f_truncate(config_file);
    }
    
    has_help = false;
    
    return true;
}

/// An integer configuration setting
stock config_integer(const id{}, const text{}, 
                     &variable, defval, minval = cellmin, maxval = cellmax,
                     const help{} = ""
                    )
{
    switch (config_state)
    {
        case config_load:
        {
            if (!inifile_getint(config_file, id, variable))
                variable = defval;
        }
        
        case config_edit, config_help:
        {
            do_help(help);
            
            if (selected() && get_keys(SCROLL1_LEFT))
                variable -= scroller_speed();
            
            if (selected() && get_keys(SCROLL1_RIGHT))
                variable += scroller_speed();
            
            variable = clamp(variable, minval, maxval);
            
            drawbg();
            draw_text(text, label_column, current_y, textcolor);
            draw_text(str(variable), value_column, current_y, textcolor);
            next_control(rowheight);
        }
        
        case config_save:
        {
            inifile_writeint(config_file, id, variable);
        }
    }
}

/// An fixed-point configuration setting
stock config_fixed(const id{}, const text{}, 
                    &Fixed:variable, Fixed: defval, Fixed: increment = FIX(0.01),
                    Fixed: minval = fix16_min, Fixed: maxval = fix16_max,
                    const help{} = ""
                  )
{
    switch (config_state)
    {
        case config_load:
        {
            if (!inifile_getfixed(config_file, id, variable))
                variable = defval;
        }
        
        case config_edit, config_help:
        {
            do_help(help);
            
            if (selected() && get_keys(SCROLL1_LEFT))
                variable -= scroller_speed() * increment;
            
            if (selected() && get_keys(SCROLL1_RIGHT))
                variable += scroller_speed() * increment;
            
            variable = fclamp(variable, minval, maxval);
            variable = increment * fround(variable / increment);
            
            drawbg();
            draw_text(text, label_column, current_y, textcolor);
            draw_text(strf(variable), value_column, current_y, textcolor);
            next_control(rowheight);
        }
        
        case config_save:
        {
            inifile_writefixed(config_file, id, variable);
        }
    }
}

/// Configuration setting with multiple textual choices
stock config_multichoice(const id{}, const text{}, 
                     &variable, const labels[]{}, defval = 0,
                     count = sizeof labels,
                     const help{} = ""
                        )
{
    switch (config_state)
    {
        case config_load:
        {
            if (!inifile_getint(config_file, id, variable))
                variable = defval;
        }
        
        case config_edit, config_help:
        {
            do_help(help);
            
            if (selected() && get_keys(SCROLL1_LEFT))
                variable--;
            
            if (selected() && get_keys(SCROLL1_RIGHT))
                variable++;
            
            if (count > 2)
                variable = clamp(variable, 0, count - 1);
            else
                variable = variable % count;
            
            drawbg();
            draw_text(text, label_column, current_y, textcolor);
            draw_text(labels[variable], value_column, current_y, textcolor);
            next_control(rowheight);
        }
        
        case config_save:
        {
            inifile_writeint(config_file, id, variable);
        }
    }
}

/// Boolean configuration setting
stock config_boolean(const id{}, const text{},
                     &bool: variable, bool: defval = false,
                     const help{} = ""
                    )
{
    new choices[]{} = ["No", "Yes"];
    new value = variable ? 1 : 0;
    config_multichoice(id, text, value, choices, defval ? 1 : 0, .help = help);
    variable = (value == 1);
}
