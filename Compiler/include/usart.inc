/** Usart serial port access.
 * serial por is alway open, so we not have open nor close
 * only read and write
*/

/// Read raw bytes from file. Returns number of bytes read.
native usart_read(buf{}, maxsize = sizeof buf);

/// Write bytes to file. Returns false if there was any error.
/// If count is -1, stops on 0 (string terminator) in src. Otherwise
/// writes exactly count bytes.
native bool: usart_write(const src{}, count = -1);

native usart_rxcount();


