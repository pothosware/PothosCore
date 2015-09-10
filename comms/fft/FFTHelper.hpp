// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include <complex>

struct kiss_fft_state;

template<typename Type>
class FFTHelper
{
    FFTHelper(int nfft, int inverse_fft);
    ~FFTHelper(void);

    void transform(const std::complex<Type> *src, std::complex<Type> *dst);

private:
    kiss_fft_state *cfg;
};
