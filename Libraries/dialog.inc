/** Simple routines for showing a question dialog to the user. */

#include <menu>
#include <draw>

/// Show a multiple choice dialog. Returns the index of the selected option, ie. an integer 0 to 3.
stock show_dialog(const message{}, opt1{}, opt2{} = "", opt3{} = "", opt4{} = "")
{
    clear_screen();
    draw_menubar(opt1, opt2, opt3, opt4);
    draw_flowtext(message, 10, 10, 380, 160);

    new result = -1;
    while (result < 0)
    {
        new button: keys = get_keys(ANY_KEY);
        if (opt1{0} && (keys & BUTTON1)) result = 0;
        if (opt2{0} && (keys & BUTTON2)) result = 1;
        if (opt3{0} && (keys & BUTTON3)) result = 2;
        if (opt4{0} && (keys & BUTTON4)) result = 3;
    }
    
    clear_screen();
    return result;
}

/// Show a yes-no question box. Returns true if user answers yes.
stock bool: show_question(const question{})
{
    return show_dialog(question, "Yes", "No") == 0;
}

/// Show a message box with OK button only.
stock show_msgbox(const message{})
{
    show_dialog(message, "OK");
}
