///
/// \file Archive/Pair.hpp
///
/// Complex support for serialization.
///
/// \copyright
/// Copyright (c) 2016 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Archive/Invoke.hpp>
#include <complex> //pair

namespace Pothos {
namespace serialization {

template<typename Archive, typename T>
void save(Archive &ar, const std::complex<T> &t, const unsigned int)
{
    ar << t.real();
    ar << t.imag();
}

template<typename Archive, typename T>
void load(Archive &ar, std::complex<T> &t, const unsigned int)
{
    T real, imag;
    ar >> real;
    ar >> imag;
    t.real(real);
    t.imag(imag);
}

template <typename Archive, typename T>
void serialize(Archive &ar, std::complex<T> &t, const unsigned int ver)
{
    Pothos::serialization::invokeSplit(ar, t, ver);
}

}}
