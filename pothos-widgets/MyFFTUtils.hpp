// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include <cmath>
#include <complex>
#include <valarray>

////////////////////////////////////////////////////////////////////////
//Window functions https://en.wikipedia.org/wiki/Window_function
//TODO more to come
////////////////////////////////////////////////////////////////////////
inline double hann(const size_t n, const size_t N)
{
    return 0.5*(1- std::cos((2.0*M_PI*n)/N-1));
}

////////////////////////////////////////////////////////////////////////
//FFT code can be foound at:
//http://rosettacode.org/wiki/Fast_Fourier_transform
////////////////////////////////////////////////////////////////////////
typedef std::complex<float> Complex;
typedef std::valarray<Complex> CArray;

// Cooley–Tukey FFT (in-place)
inline void fft(CArray& x)
{
    const size_t N = x.size();
    if (N <= 1) return;

    // divide
    CArray even = x[std::slice(0, N/2, 2)];
    CArray  odd = x[std::slice(1, N/2, 2)];

    // conquer
    fft(even);
    fft(odd);

    // combine
    for (size_t k = 0; k < N/2; ++k)
    {
        Complex t = std::polar(1.0f, -2 * float(M_PI) * k / N) * odd[k];
        x[k    ] = even[k] + t;
        x[k+N/2] = even[k] - t;
    }
}

// inverse fft (in-place)
inline void ifft(CArray& x)
{
    // conjugate the complex numbers
    x = x.apply(std::conj);

    // forward fft
    fft( x );

    // conjugate the complex numbers again
    x = x.apply(std::conj);

    // scale the numbers
    x /= x.size();
}

////////////////////////////////////////////////////////////////////////
//FFT Power spectrum
////////////////////////////////////////////////////////////////////////
inline std::valarray<float> fftPowerSpectrum(CArray &fftBins)
{
    //windowing
    float windowPower(0.0);
    for (size_t n = 0; n < fftBins.size(); n++)
    {
        double w_n = hann(n, fftBins.size());
        windowPower += w_n*w_n;
        fftBins[n] *= w_n;
    }
    windowPower /= fftBins.size();

    //take fft
    fft(fftBins);

    //power calculation
    std::valarray<float> powerBins(fftBins.size());
    for (size_t i = 0; i < fftBins.size(); i++)
    {
        powerBins[i] = std::norm(fftBins[i]);
        powerBins[i] = 10*std::log10(powerBins[i]);
        powerBins[i] -= 20*std::log10(fftBins.size());
        powerBins[i] -= 10*std::log10(windowPower);
    }

    //bin reorder
    for (size_t i = 0; i < powerBins.size()/2; i++)
    {
        std::swap(powerBins[i], powerBins[i+powerBins.size()/2]);
    }

    return powerBins;
}
