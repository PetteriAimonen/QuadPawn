/** Implementation of some Fourier transform algorithms. These typically
 * take an input of 8-bit packed arrays, such as the ones you get from
 * the wavein module. Output is in the fixed point format.
 */

#include <fixed>
#include <complex>
 
/// Discrete Fourier transform at a single frequency.
/// The frequency is given as the number of samples in a single period.
/// Count is the number of samples available in input. It will be rounded
/// down to the nearest multiple of period.
native dft(const input{}, &Fixed: real, &Fixed: imag, Fixed: period, count);

/// Fast Fourier transform.
/// Transform length (count) must be a power of 2.
/// All arrays are of same size.
/// If window is true, the Hamming window function is applied to input values before processing.
native fft(const input{}, Fixed: real[], Fixed: imag[], count, bool:window = true);
