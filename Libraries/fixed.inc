/** This is a fixed point math library, using the libfixmath fix16_t type.
 * PAWN has also its own fixed point implementation, but that scales by a
 * decimal constant. The libfixmath implementation is somewhat faster and
 * includes sin and cos.
 *
 * The multiplication, division, addition and subtraction operations are
 * overflow protected, i.e. they saturate to min/max value.
 */

#include <core>
 
/// For defining fractional literals, you need to use the FIX macro, like this:
/// new Fixed:foo = FIX(1.234)
#pragma rational Decimal(5)
#define FIX(%1) (Fixed:(_:(%1 * Decimal:65536 + Decimal:50000) / 100000))

// Forbid unimplemented operations on the internal "Decimal" type
forward operator+(Decimal:oper1, oper2);
forward operator-(Decimal:oper1, oper2);
forward operator-(oper1, Decimal:oper2);
forward operator*(Decimal:oper1, oper2);
forward operator/(Decimal:oper1, oper2);
forward operator/(oper1, Decimal:oper2);

const Fixed: pi = FIX(3.14159);
const Fixed: exp_base = FIX(2.71828);
const Fixed: fone = FIX(1.0);
const Fixed: log_of_10 = FIX(2.30259);
const Fixed: fix16_min = Fixed:cellmin;
const Fixed: fix16_max = Fixed:cellmax;

/// Integer to fixed point
native Fixed:fixed(value);

/// Fixed point to integer
native fround(Fixed:value);

/// Mathematical operators for Fixed.
native Fixed:fadd(Fixed:oper1, Fixed:oper2);
native Fixed:fsub(Fixed:oper1, Fixed:oper2);
native Fixed:fmul(Fixed:oper1, Fixed:oper2);
native Fixed:fdiv(Fixed:dividend, Fixed:divisor);
native Fixed:operator+(Fixed:oper1, Fixed:oper2) = fadd;
native Fixed:operator-(Fixed:oper1, Fixed:oper2) = fsub;
native Fixed:operator*(Fixed:oper1, Fixed:oper2) = fmul;
native Fixed:operator/(Fixed:oper1, Fixed:oper2) = fdiv;
native Fixed:operator=(oper) = fixed;
stock Fixed:operator++(Fixed:oper) return oper + fixed(1);
stock Fixed:operator--(Fixed:oper) return oper - fixed(1);
stock Fixed:operator*(Fixed:oper1, oper2) return Fixed: (_:oper1 * oper2);
stock Fixed:operator/(Fixed:oper1, oper2) return oper1 / fixed(oper2);
stock Fixed:operator/(oper1, Fixed:oper2) return fixed(oper1) / oper2;
stock Fixed:operator+(Fixed:oper1, oper2) return oper1 + fixed(oper2);
stock Fixed:operator-(Fixed:oper1, oper2) return oper1 - fixed(oper2);
stock Fixed:operator-(oper1, Fixed:oper2) return fixed(oper1) - oper2;
stock bool:operator>(Fixed:oper1, oper2)  return oper1 > fixed(oper2);
stock bool:operator>(oper1, Fixed:oper2)  return fixed(oper1) > oper2;
stock bool:operator>=(Fixed:oper1, oper2) return oper1 >= fixed(oper2);
stock bool:operator>=(oper1, Fixed:oper2) return fixed(oper1) >= oper2;
stock bool:operator<(Fixed:oper1, oper2)  return oper1 < fixed(oper2);
stock bool:operator<(oper1, Fixed:oper2)  return fixed(oper1) < oper2;
stock bool:operator<=(Fixed:oper1, oper2) return oper1 <= fixed(oper2);
stock bool:operator<=(oper1, Fixed:oper2) return fixed(oper1) <= oper2;
stock bool:operator==(Fixed:oper1, oper2) return oper1 == fixed(oper2);
stock bool:operator!=(Fixed:oper1, oper2) return oper1 != fixed(oper2);
forward operator%(Fixed:oper1, oper2); // Forbidden
forward operator%(oper1, Fixed:oper2); // Forbidden

/// Integer part of a number
stock ftrunc(Fixed:value) return (_:value) / 65536;

/// Fractional part of a number
stock Fixed:frac(Fixed: value) return Fixed:(abs(_:value) & 0xFFFF);

/// Exponent function
native Fixed:exp(Fixed:exponent);

/// Square root
native Fixed:sqrt(Fixed:value);

/// Convert radians to degrees
stock Fixed:rad2deg(Fixed:radians) return radians * FIX(180.0) / pi;

/// Convert degrees to radians
stock Fixed:deg2rad(Fixed:degrees) return degrees * pi / FIX(180.0);

/// Sine function
native Fixed:sin(Fixed:radians);

/// Cosine function
stock Fixed:cos(Fixed:radians) return sin(radians + pi/2);

/// Tangent function
stock Fixed:tan(Fixed:radians) return sin(radians) / cos(radians);

/// Inverse of tangent
native Fixed:atan2(Fixed:y, Fixed:x);

/// Inverse of sine
stock Fixed:asin(Fixed: value) return atan2(value, sqrt(fone - value * value));

/// Inverse of cosine
stock Fixed:acos(Fixed: value) return atan2(sqrt(fone - value * value), value);

/// Raising to power for integer exponents
stock Fixed:pow(Fixed:value, exponent)
{
    if (exponent == 0)
    {
        return FIX(1.0);
    }
    else if (exponent > 0)
    {
        new Fixed: result = FIX(1.0);
        while (exponent)
        {
            result *= value;
            exponent--;
        }
        return result;
    }
    else
    {
        new Fixed: result = FIX(1.0);
        while (exponent)
        {
            exponent++;
            result /= value;
        }
        return result;
    }
}

/// Calculate natural logarithm of value
native Fixed:log(Fixed: value);

/// Calculate base-10 logarithm
stock Fixed:log10(Fixed: value)
{
    return log(value) / log_of_10;
}

/// Calculate base-10 logarithm of an integer value
stock Fixed:log10i(value)
{
    return (log(Fixed:value) - FIX(11.09035)) / log_of_10;
}

/// Absolute value of a fixed point number
stock Fixed: absf(Fixed: value)
{
    if (value < 0)
        return -value;
    else
        return value;
}

/// Clamp value to range
stock Fixed: fclamp(Fixed: value, Fixed: min = fix16_min, Fixed: max = fix16_max)
{
    if (value < min)
        return min;
    else if (value > max)
        return max;
    else
        return value;
}

