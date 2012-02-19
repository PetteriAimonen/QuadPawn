/** The menu bar contains bindings for the buttons. You can use these
 * functions to set what happens when you press a particular button.
 *
 * The callback functions should be declared public, and their name should
 * be given as an argument to the set_button* function.
 */

/// Set the action for left-most button. Label is the text to show on
/// screen (max. 5 characters) and function is name of the public function
/// to call when the button is pressed. The function will receive 0 params.
native set_button1(label{}, function{});

native set_button2(label{}, function{});

native set_button3(label{}, function{});

/// Set the action for the plus-minus scroller key. The function will receive
/// 1 parameter, which is the number of scrollings happened. The firmware
/// has an internal speed-up factor, so if the user holds down the button
/// you'll start to receive events of e.g. +-50 scrolls.
native set_scroll1(function{});

native set_scroll2(function{});

/// Set the action for pressing down the scroller. The function will receive
/// 0 params.
native set_scrollpress1(function{});
native set_scrollpress2(function{});

/// Redraw the menu bar. You don't usually need to call this yourself.
native redraw_menu();

/// Dispatch any events that have occurred in the meantime. You should call
/// this periodically if you want to receive button events.
native doevents();
