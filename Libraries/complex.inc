/** Simple complex number manipulation routines. */

#include <fixed>

/// Get the absolute value from a complex number
stock Fixed: cabs(Fixed: real, Fixed: imag)
{
    return sqrt(real * real + imag * imag);
}

/// Get the argument (phase) from a complex number, in radians
stock Fixed: carg(Fixed: real, Fixed: imag)
{
    return atan2(imag, real);
}

/// Multiply complex numbers
stock cmul(&Fixed: real1, &Fixed: imag1, Fixed: real2, Fixed: imag2)
{
    // (a + ib) * (c + id) = ac - bd + i(bc + ad)
    new Fixed: tmp = real1 * real2 - imag1 * imag2;
    imag1 = imag1 * real2 + imag2 * real1;
    real1 = tmp;
}

/// Divide complex numbers
stock cdiv(&Fixed: real1, &Fixed: imag1, Fixed: real2, Fixed: imag2)
{
    // (a + ib) / (c + id) = (a + ib) (c - id) / (c^2 + d^2)
    new Fixed: divisor = (real2 * real2 + imag2 * imag2);
    cmul(real1, imag1, real2, -imag2);
    
    real1 = real1 / divisor;
    imag1 = imag1 / divisor;
}
