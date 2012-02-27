/** The menu bar contains bindings for the buttons. You can use these
 * functions to set what happens when you press a particular button.
 *
 * The callback functions should be declared public, and their name should
 * be given as an argument to the set_button* function.
 *
 * To dispatch events, you must call sleep(x) periodically. X is the number
 * of milliseconds to sleep, 0 to just dispatch events.
 */

/// Set the action for the three first buttons. The fourth button is reserved
/// for the system to abort the program. Label is the text to show on
/// screen (max. 7 characters) and function is name of the public function
/// to call when the button is pressed. The function will receive 0 params.
native set_buttons(label1{} = "", function1{} = "",
                   label2{} = "", function2{} = "",
                   label3{} = "", function3{} = "");

/// Set the action for the scroller keys. The functions will receive
/// 1 parameter, which is the number of scrollings happened. The firmware
/// has an internal speed-up factor, so if the user holds down the button
/// you'll start to receive events of e.g. +-50 scrolls.
/// When the scroller is pressed, the function is called with parameter '0'.
native set_scrollers(function1{} = "", function2{} = "");

/// Redraw the menu bar. You don't usually need to call this yourself.
native redraw_menu();

/// Push the current configuration in a virtual stack. This can be used to
/// save old state when displaying a dialog temporarily. Clears the settings
/// after pushing. There are 4 stack levels available, so you can call this
/// function at most 3 times.
native push_menu();

/// Restore previously pushed settings
native pop_menu();
