// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include "PlotUtilsConfig.hpp"
#include <cmath>
#include <complex>
#include <valarray>
#include <string>
#include <cassert>
#include <algorithm>
#include <spuce/filters/design_window.h>

////////////////////////////////////////////////////////////////////////
//FFT code can be found at:
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
struct FFTPowerSpectrum
{
    void setWindowType(const std::string &windowType, const std::vector<double> &windowArgs)
    {
        _windowType = windowType;
        _windowArgs = windowArgs;
        _precomputedWindow.clear();
    }

    std::valarray<float> transform(CArray &fftBins)
    {
        //windowing
        if (_precomputedWindow.size() != fftBins.size())
        {
            _precomputedWindow = spuce::design_window(_windowType, fftBins.size(), _windowArgs.empty()?0.0:_windowArgs.at(0));
            _precomputedWindowPower = 0.0;
            for (size_t n = 0; n < _precomputedWindow.size(); n++)
            {
                _precomputedWindowPower += _precomputedWindow[n]*_precomputedWindow[n];
            }
            _precomputedWindowPower = std::sqrt(_precomputedWindowPower/_precomputedWindow.size());
        }
        for (size_t n = 0; n < fftBins.size(); n++) fftBins[n] *= _precomputedWindow[n];

        //take fft
        fft(fftBins);

        //window and fft gain adjustment
        const float gain_dB = 20*std::log10(fftBins.size()) + 20*std::log10(_precomputedWindowPower);

        //power calculation
        std::valarray<float> powerBins(fftBins.size());
        for (size_t i = 0; i < fftBins.size(); i++)
        {
            const float norm = std::max(std::norm(fftBins[i]), 1e-20f);
            powerBins[i] = 10*std::log10(norm) - gain_dB;
        }

        //bin reorder
        for (size_t i = 0; i < powerBins.size()/2; i++)
        {
            std::swap(powerBins[i], powerBins[i+powerBins.size()/2]);
        }

        return powerBins;
    }

    std::string _windowType;
    std::vector<double> _windowArgs;
    std::vector<double> _precomputedWindow;
    double _precomputedWindowPower;
};
