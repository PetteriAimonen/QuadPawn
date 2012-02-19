/** A basic string library.
 * We only deal with packed strings here in order to conserve code space.
 */

#include <fixed>
 
native strlen(const string[]);

/// Add source string to the end of the destination string
stock strcat(dest{}, const source{}, maxlength=sizeof dest)
{
    maxlength *= 4;
    maxlength--; // Space for terminating zero

    new pos = 0;
    while (dest{pos} != 0 && maxlength--) pos++;

    new pos2 = 0;
    while (source{pos2} != 0 && maxlength--)
        dest{pos++} = source{pos2++};
    
    dest{pos} = 0;
}

/// Copy source string to dest
stock strcpy(dest{}, const source{}, maxlength=sizeof dest)
{
    maxlength *= 4;
    maxlength--;
    new pos = 0;
    while (source{pos} != 0 && maxlength--)
    {
        dest{pos} = source{pos};
        pos++;
    }
    
    dest{pos} = 0;
}

/// Convert string to integer, starting at string[index].
/// Writes the position of the first non-numeric character in endindex.
native strval(const string[], index = 0, &endindex = 0, base = 10);

/// Convert integer to string, returns the number of characters.
native valstr(dest[], value, maxlength = sizeof dest, base = 10);

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
    
    new pos = valstr(result, ftrunc(value)); 
    
    if (pos == 0)
    {
        result{pos} = '0';
        pos++;
    }
    
    result{pos} = '.';
    pos++;
    
    new scale = 1;
    for (new i = 0; i < decimals; i++) scale *= 10;
    
    new fracpart{8};
    new count = valstr(fracpart, fround(frac(value) * scale));
    
    // Add any missing zeros in between
    while (count < decimals)
    {
        decimals--;
        result{pos} = '0';
        pos++;
    }
    
    // Add the decimals
    new i = 0;
    while (decimals > 0)
    {
        result{pos} = fracpart{i};
        pos++;
        i++;
        decimals--;
    }
    
    return result;
}

// native strformat(dest[], size=sizeof dest, bool:pack=true, const format[], {Fixed,Float,_}:...);

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

