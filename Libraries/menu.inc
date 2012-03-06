/** The menu bar appears at the top of the screen and contains labels for
 * button actions. This file also declares event callbacks for buttons,
 * which are an alternative interface relative to the polling functions in
 * buttons.inc.
 */

/// Set the menu labels, there is space for about 7 characters on the screen.
native draw_menubar(label1{}, label2{}, label3{}, label4{});

/// Implement these functions to handle the buttons
forward @button1();
forward @button2();
forward @button3();
forward @button4();

