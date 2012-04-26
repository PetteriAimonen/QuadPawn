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

/// Locate a character in a string. Returns position as 0-based index, or -1
/// if not found.
stock strchr(const str{}, character, start = 0)
{
    new pos = start;
    while (str{pos} != 0)
    {
        if (str{pos} == character) return pos;
        pos++;
    }
    return -1;
}

/// Locate the last instance of a character in a string.
/// Returns position as 0-based index, or -1 if not found.
stock strrchr(const str{}, character, end = cellmax)
{
    new pos = 0;
    new result = -1;
    while (str{pos} != 0 && pos < end)
    {
        if (str{pos} == character) result = pos;
        pos++;
    }
    return result;
}

/// Get the middle portion of a string
/// "foobar", 1, 3  =>  "oob"
stock strmid(dest{}, const str{}, start, length)
{
    for (new i = 0; i < length; i++)
    {
        dest{i} = str{i + start};
        if (dest{i} == 0) break;
    }
    dest{length} = 0;
}

/// Pad a string to given length by prepending characters.
stock strpad(dest{}, length, chr = ' ')
{
    new slen = strlen(dest);
    new count = length - slen;
    if (count <= 0) return;

    for (new i = slen; i >= 0; i--)
        dest{i + count} = dest{i};
    
    for (new i = 0; i < count; i++)
        dest{i} = chr;
}

/// Split the string to two parts by some separator. The initial part is
/// moved to dest and remaining part is left in the source. The separator
/// is removed. Returns false if separator is not found or dest is too small.
/// If coalesce is true, repeated separators are regarded as one.
stock bool: split(dest{}, source{}, separator{}=" ", bool:coalesce = true,
maxlength=sizeof dest)
{
    maxlength *= 4;
    maxlength--;
    
    new pos = 0;
    while (strchr(separator, source{pos}) == -1)
    {
        if (source{pos} == 0 || maxlength-- < 0)
        {
            dest{0} = 0;
            return false;
        }
    
        dest{pos} = source{pos};
        pos++;
    }
    
    dest{pos} = 0;
    
    // Remove separator(s)
    pos++;
    if (coalesce)
    {
        while (strchr(separator, source{pos}) != -1)
            pos++;
    }
    
    new pos2 = 0;
    while (source{pos} != 0)
        source{pos2++} = source{pos++};
    
    source{pos2} = 0;
    return true;
}

/// Remove any occurrence of chars from the end of string.
stock rstrip(dest{}, chars{}=" ")
{
    new pos = strlen(dest) - 1;
    
    while (strchr(chars, dest{pos}) != -1)
        dest{pos--} = 0;
}

/// Remove any occurrence of chars from the beginning of string.
stock lstrip(dest{}, chars{}=" ")
{
    new pos = 0;
    
    while (strchr(chars, dest{pos}) != -1)
        pos++;
    
    new pos2 = 0;
    while (dest{pos} != 0)
        dest{pos2++} = dest{pos++};
    
    dest{pos2} = 0;
}

/// Remove any occurrence of chars from either end of string
stock strip(dest{}, chars{}=" ")
{
    lstrip(dest, chars);
    rstrip(dest, chars);
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

/// Return integer converted to hex string, shorthand for strval
stock hex(value)
{
    new dest{12};
    valstr(dest, value, .base = 16);
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
    
    new scale = 1;
    for (new i = 0; i < decimals; i++) scale *= 10;
    
    new intval = ftrunc(value);
    new fracval = fround(frac(value) * scale);
    
    if (fracval >= scale)
    {
        intval++;
        fracval = 0;
    }
    
    if (intval == 0 && value < 0)
    {
        result = "-0.";
    }
    else
    {
        valstr(result, intval); 
        strcat(result, ".");
    }
    
    new fracpart{8};
    valstr(fracpart, fracval, .minlength = decimals);
    strcat(result, fracpart);
    
    return result;
}

/// Return string converted to fixed point value
stock Fixed: strfixed(const str{})
{
    new pos = 0;
    new intpart = strval(str, .endindex = pos);
    
    new fracpart = 0;
    
    if (str{pos} == '.' || str{pos} == ',')
    {
        new tmp{5};
        strmid(tmp, str, pos + 1, 4);
        while (strlen(tmp) < 4) strcat(tmp, "0");
        
        fracpart = strval(tmp);
    }
    
    new Fixed: result = fixed(intpart);
    result = result + (Fixed:fracpart) * FIX(6.5536);
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

