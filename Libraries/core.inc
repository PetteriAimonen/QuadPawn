native heapspace();

native funcidx(const name[]);

native numargs();
native getarg(arg, index=0);
native setarg(arg, index=0, value);

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
