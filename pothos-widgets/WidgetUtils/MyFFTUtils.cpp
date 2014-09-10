// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "MyFFTUtils.hpp"
#include <QString>
#include <cmath>
#include <stdexcept>

static double rectangular(const size_t, const size_t)
{
    return 1.0;
}

static double hann(const size_t n, const size_t M)
{
    return 0.5 - 0.5*std::cos((2.0*M_PI*n)/(M-1));
}

static double hamming(const size_t n, const size_t M)
{
    return 0.54 - 0.46*std::cos((2.0*M_PI*n)/(M-1));
}

static double blackman(const size_t n, const size_t M)
{
    return 0.42 - 0.5*std::cos((2.0*M_PI*n)/M) + 0.08*std::cos((4.0*M_PI*n)/M);
}

static double bartlett(const size_t n, const size_t M)
{
    return (2.0/(M-1.0)) * (((M-1.0)/2.0) - std::abs(n - ((M-1.0)/2.0)));
}

static double flattop(const size_t n, const size_t M)
{
    static const double a0 = 1.0, a1 = 1.93, a2 = 1.29, a3 = 0.388, a4=0.028;
    return a0
        -a1*std::cos((2.0*M_PI*n)/(M-1))
        +a2*std::cos((4.0*M_PI*n)/(M-1))
        -a3*std::cos((6.0*M_PI*n)/(M-1))
        +a4*std::cos((8.0*M_PI*n)/(M-1));
}

WindowFunction::WindowFunction(void):
    _calc(&hann),
    _power(1.0f)
{
    return;
}

void WindowFunction::setType(const std::string &name_)
{
    const auto name = QString::fromStdString(name_).toLower();
    if (name == "rectangular") _calc = &rectangular;
    else if (name == "hann") _calc = &hann;
    else if (name == "hamming") _calc = &hamming;
    else if (name == "blackman") _calc = &blackman;
    else if (name == "bartlett") _calc = &bartlett;
    else if (name == "flattop") _calc = &flattop;
    else throw std::runtime_error("WindowFunction::setType("+name_+")");
    this->reload();
}

void WindowFunction::setSize(const size_t length)
{
    if (length == _window.size()) return;
    _window.resize(length);
    this->reload();
}

void WindowFunction::reload(void)
{
    _power = 0.0;
    const auto length = _window.size();
    for (size_t n = 0; n < length; n++)
    {
        _window[n] = _calc(n, length);
        _power += _window[n]*_window[n];
    }
    _power = std::sqrt(_power/length);
}
