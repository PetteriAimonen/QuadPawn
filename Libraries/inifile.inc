/** Very basic inifile support.
 * Does not support sections or random-access write (whole file must be
 * written at once).
 */

#include <file>
#include <string>
#include <console>
#include <fixed>

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
    for (new i = strlen(optname); i < 20; i++)
        f_write(file, " ");
    
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

/// Read a fixed point value
stock bool:inifile_getfixed(File: file, const optname{}, &Fixed: value)
{
    new tmp{10};
    if (!inifile_get(file, optname, tmp)) return false;
    
    value = strfixed(tmp);
    return true;
}

/// Write a fixed point value
stock inifile_writefixed(File: file, const optname{}, Fixed: value)
{
    inifile_write(file, optname, strf(value));
}

/// Read a fixed point array
stock bool:inifile_getfarray(File: file, const optname{}, Fixed: value[], count = sizeof value)
{
    new tmp{256}, tmp2{16};
    if (!inifile_get(file, optname, tmp)) return false;
    
    for (new i = 0; i < count - 1; i++)
    {
        if (!split(tmp2, tmp)) return false;
        value[i] = strfixed(tmp2);
    }
    value[count - 1] = strfixed(tmp);
    
    return true;
}

/// Write a fixed point array
stock inifile_writefarray(File: file, const optname{}, const Fixed: value[], count = sizeof value)
{
    new tmp{256} = "", tmp2{16};
    
    for (new i = 0; i < count; i++)
    {
        tmp2 = strf(value[i]);
        strpad(tmp2, 10);
        strcat(tmp, tmp2);
        strcat(tmp, " ");
    }
    
    inifile_write(file, optname, tmp);
}

