/** Filesystem access. Note that all files are opened in 'binary'
 * mode, so to write Windows-style newlines, you have to use \r\n.
 *
 * The number of open files at a time is limited to 4. Remember to close
 * the files you open.
 */

/// Flags to specify the mode when opening a file.
/// Flags can be combined with the | operator.
const FileAccess:
{
    /// Allow reading from the file
    FA_READ             = 0x01,
    
    /// Allow writing to the file (can be combined with FA_READ)
    FA_WRITE            = 0x02,
    
    /// Open an existing file and fail if the file does not exist (default)
    FA_OPEN_EXISTING    = 0x00,
    
    /// Create a new file and fail if the file already exists.
    FA_CREATE_NEW       = 0x04,
    
    /// Create a new file or truncate (overwrite) an existing file.
    FA_CREATE_ALWAYS    = 0x08,
    
    /// Open an existing file, or if it does not exist, create one.
    /// Can be used with f_lseek() to implement "append".
    FA_OPEN_ALWAYS      = 0x10
}

/// Open a file. Returns 0 if the operation fails, and a non-zero file id
/// otherwise.
native File: f_open(const filename{}, FileAccess: mode = FA_READ);

/// Close a file and release any RAM allocated for it.
/// Returns true if everything succeeds.
/// If parameter is 0, does nothing and returns false.
native bool: f_close(File: file);

/// Read raw bytes from file. Returns number of bytes read, which is
/// less than count only in the case of EOF or error.
native f_read(File: file, dest{}, count);

/// Write bytes to file. Returns false if there was any error.
/// If count is -1, stops on 0 (string terminator) in src. Otherwise
/// writes exactly count bytes.
native bool: f_write(File: file, const src{}, count = -1);

/// Read a line from file. The result string will not include the trailing
/// \r or \n. If the buffer is too small, returns 'false' and rest of the
/// line will be returned on next call.
stock bool: f_readline(File: file, dest{}, destsize = sizeof dest)
{
    new i;
    destsize = destsize * 4 - 1;
    for (i = 0; i < destsize; i++)
    {
        new b{1};
        if (f_read(file, b, 1) != 1) break;
        
        if (b{0} == '\r') continue;
        if (b{0} == '\n')
        {
            dest{i} = 0;
            return true;
        }
        
        dest{i} = b{0};
    }
    
    dest{i} = 0;
    return false;
}

/// Seek into a different point in file, pos being count of bytes from
/// the start. Specifying a pos beyond end of file expands the file.
native bool: f_lseek(File: file, position);

/// Get the current position in a file.
native f_tell(File: file);

/// Get the size of the file
native f_size(File: file);

/// Truncate file at current location
native bool:f_truncate(File: file);

/// Remove a file
native f_unlink(const filename{});

/// Get free space in bytes
native f_getfree();

/// Open the root directory for listing files.
/// Note that the fatfs implementation is stripped down and doesn't support
/// subdirectories.
native f_opendir();

/// Get the next file from directory
/// Give a variable reference as filesize to retrieve also the file size.
/// Returns false when there are no more files.
native bool: f_readdir(filename{13}, &filesize = 0);
