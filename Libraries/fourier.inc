/** Implementation of some Fourier transform algorithms. These typically
 * take an input of 8-bit packed arrays, such as the ones you get from
 * the wavein module. Output is in the fixed point format.
 */

#include <fixed>
 
/// Discrete Fourier transform at a single frequency.
/// The frequency is given as the number of samples in a single period.
/// Count is the number of samples available in input. It will be rounded
/// down to the nearest multiple of period.
native dft(input{}, &Fixed: real, &Fixed: imag, Fixed: period, count);

/// Get the absolute value from a complex number
stock Fixed: cabs(Fixed: real, Fixed: imag)
{
    return sqrt(real * real + imag * imag);
}

/// Get the argument (phase) from a complex number, in radians
stock Fixed: carg(Fixed: real, Fixed: imag)
{
    return atan2(real, imag);
}
