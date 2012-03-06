/** A basic string library.
 * We only deal with packed strings here in order to conserve code space.
 */

#include <fixed>
 
native strlen(const string{});

/// Add source string to the end of the destination string
stock strcat(dest{}, const source{}, maxlength=sizeof dest)
{
    maxlength *= 4;
    maxlength--; // Space for terminating zero

    new pos = strlen(dest);
    maxlength -= pos;
    
    new pos2 = 0;
    while (source{pos2} != 0 && maxlength-- > 0)
        dest{pos++} = source{pos2++};
    
    dest{pos} = 0;
}

/// Copy source string to dest
stock strcpy(dest{}, const source{}, maxlength=sizeof dest)
{
    dest{0} = 0;
    strcat(dest, source, maxlength);
}

/// Convert string to integer, starting at str{index}.
/// Writes the position of the first non-numeric character in endindex.
native strval(const str{}, index = 0, &endindex = 0, base = 10);

/// Convert integer to string, returns the number of characters.
native valstr(dest[], value, maxlength = sizeof dest, base = 10, minlength = 0);

/// Return integer converted to string, shorthand for strval
stock str(value)
{
    new dest{12};
    valstr(dest, value);
    return dest;
}

/// Return fixed point value converted to string, with up to 4 decimals
stock strf(Fixed: value, decimals = 4)
{
    new result{12};
    
    if (decimals == 0)
    {
        valstr(result, fround(value));
        return result;
    }
    
    valstr(result, ftrunc(value)); 
    strcat(result, ".");
    
    new scale = 1;
    for (new i = 0; i < decimals; i++) scale *= 10;
    
    new fracpart{8};
    valstr(fracpart, fround(frac(value) * scale), .minlength = decimals);
    strcat(result, fracpart);
    
    return result;
}

stock tolower(c)
{
    if ('A' <= c <= 'Z')
        return c + ('a' - 'A');
    else
        return c;
}

stock strtolower(c{})
{
    for (new i = 0; c{i} != 0; i++)
        c{i} = tolower(c{i});
}

stock toupper(c)
{
    if ('a' <= c <= 'z')
        return c - ('a' - 'A');
    else
        return c;
}

stock strtoupper(c{})
{
    for (new i = 0; c{i} != 0; i++)
        c{i} = toupper(c{i});
}

stock strcmp(const string1{}, const string2{}, bool:ignorecase=false, length=cellmax)
{
    for (new i = 0; i < length; i++)
    {
        new c1 = string1{i};
        new c2 = string2{i};
        
        if (ignorecase)
        {
            c1 = tolower(c1);
            c2 = tolower(c2);
        }
    
        if (c1 == c2)
        {
            if (c1 == 0)
                return 0;
            else
                continue;
        }
        
        if (c1 < c2)
            return -1;
        else
            return 1;
    }
    return 0;
}

stock bool: strequal(const string1{}, const string2{}, bool:ignorecase=false, length=cellmax)
    return strcmp(string1, string2, ignorecase, length) == 0;

