/** Functions based on internal 1ms timer tick. */

/// Return time passed since system boot in milliseconds.
/// Overflows after 24 days of operation.
native get_time();

/// Implement this function to get periodic callbacks when
/// in "event driven" mode.
forward @timertick();

/// Set the period for the timertick callback, in milliseconds.
/// Set to 0 (default) to disable the timer tick.
native set_timer(period);

/// Delay without yielding events. This simply waits in a busy loop.
native delay_ms(delay);
