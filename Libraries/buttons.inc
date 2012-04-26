/** This is a basic library to read the buttons and scrollers of DSO Quad
 * in a polling manner. For an event-driven approach, see menu.inc.
 *
 * Both approaches can be used in a single program: the polling approach
 * has priority over event-driven, as long as it doesn't call sleep().
 *
 * The event-driven approach is more useful for the main program, whereas
 * polling approach is easier in modal dialogs and configuration screens.
 *
 * Both approaches use the same internal key buffer, which stores any
 * keypresses even if the application is not reading the keys at the
 * moment. When exiting a dialog, you may want to use get_keys(ANY_KEY)
 * to clear the buffer, so that no spurious keypresses are emitted to
 * the main application.
 *
 * In addition to keypresses (which are flagged when the key first goes down),
 * an application may be interested in keys that are being held down. Holding
 * a key does not re-flag the key buffer, but you can use held_keys() to get
 * the information while the key is still down.
 */

/// Flags for querying the status of buttons. Multiple flags can
/// be combined using | operator.
const button:
{
    BUTTON1             = 0x4000,
    BUTTON2             = 0x2000,
    BUTTON3             = 0x0100,
    BUTTON4             = 0x0200,
    
    SCROLL1_LEFT        = 0x0400,
    SCROLL1_RIGHT       = 0x0800,
    SCROLL1_PRESS       = 0x1000,
    
    SCROLL2_LEFT        = 0x0008,
    SCROLL2_RIGHT       = 0x8000,
    SCROLL2_PRESS       = 0x0040,
    
    ANY_KEY             = 0xFFFF
}

/// Check if some keys have been pressed, but don't clear the key status.
/// Returns the keys that were pressed or 0 if no buttons were pressed.
native button: peek_keys(button: keys);

/// Check if some keys have been pressed, and clear the status for them.
native button: get_keys(button: keys);

/// Check if some keys are being held down.
/// Returns the keys that are down and writes to variable the number of
/// milliseconds they've been down. Pressing another button while one key
/// is down resets the count.
native button: held_keys(button: keys, &milliseconds = 0);

/// Get the scroller increment based on held_keys time. This achieves
/// incremental acceleration.
native scroller_speed();

#include <time>

/// Wait the specified number of milliseconds, but abort the wait if a button
/// is pressed.
stock wait_keys(button: keys, milliseconds)
{
    new start = get_time();
    while (get_time() - start < milliseconds && !peek_keys(keys)) {}
}
