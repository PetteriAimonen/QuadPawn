/** Very basic inifile support.
 * Does not support sections or random-access write (whole file must be
 * written at once).
 */

#include <file>
#include <string>
#include <console>

/// Read a setting from the ini file. Returns false if the setting was not
/// found.
stock bool:inifile_get(File: file, const optname{}, value{}, maxlength = sizeof value)
{
    new optbuf{32};
    new linebuf{200};
    new bool: rewound = false;
    while (f_readline(file, linebuf) || !rewound)
    {
        split(optbuf, linebuf, "=");
        strip(optbuf);
        
        if (strequal(optbuf, optname))
        {
            strip(linebuf);
            strcpy(value, linebuf, maxlength);
            return true;
        }
        
        if (!rewound)
        {
            // The options are not in order, restart from beginning of file.
            f_lseek(file, 0);
            rewound = true;
        }
    }
    
    return false;
}

/// Write a setting to the ini file.
stock inifile_write(File: file, const optname{}, const value{})
{
    f_write(file, optname);
    f_write(file, " = ");
    f_write(file, value);
    f_write(file, "\r\n");
}

/// Read an integer
stock bool:inifile_getint(File: file, const optname{}, &value)
{
    new tmp{10};
    if (!inifile_get(file, optname, tmp)) return false;
    
    value = strval(tmp);
    return true;
}

/// Write an integer
stock inifile_writeint(File: file, const optname{}, value)
{
    inifile_write(file, optname, str(value));
}
