///
/// \file Util/MathCompat.hpp
///
/// Provide math compatibility functions and definitions.
///
/// \copyright
/// Copyright (c) 2014-2015 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Config.hpp>

#include <cmath>

#if defined(_MSC_VER) && (_MSC_VER <= 1700)

namespace std
{
    template <typename T>
    T round(const T x)
    {
        return (x < T(0.0)) ? std::ceil(x - T(0.5)) : std::floor(x + T(0.5));
    }

    template <typename T>
    long int lround(const T x)
    {
        return (long int)((x < T(0.0)) ? (x - T(0.5)) : (x + T(0.5)));
    }

    template <typename T>
    long long int llround(const T x)
    {
        return (long long int)((x < T(0.0)) ? (x - T(0.5)) : (x + T(0.5)));
    }

    template <typename T>
    T log2(const T x)
    {
        return std::log(x)/std::log(2.0);
    }
}

#endif
