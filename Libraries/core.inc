/// Return number of free cells in the virtual machine heap
native heapspace();

/// Return index of a public function.
native funcidx(const name[]);

/// Variable argument list handling
native numargs();
native getarg(arg, index=0);
native setarg(arg, index=0, value);

/// Clear an array
native memset(array[], value = 0, count = sizeof array);

stock min(value1, value2)
    return (value1 <= value2) ? value1 : value2;

stock max(value1, value2)
    return (value1 >= value2) ? value1 : value2;

stock clamp(value, min=cellmin, max=cellmax)
{
    if (value < min)
        return min;
    else if (value > max)
        return max;
    else
        return value;
}

stock abs(value)
{
    if (value < 0)
        return -value;
    else
        return value;
}
