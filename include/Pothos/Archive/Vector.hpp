///
/// \file Archive/Vector.hpp
///
/// Vector support for serialization.
///
/// \copyright
/// Copyright (c) 2016 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Archive/Invoke.hpp>
#include <Pothos/Archive/Numbers.hpp>
#include <utility> //move
#include <vector>

namespace Pothos {
namespace serialization {

//------------ boolean vectors use a special internal type --------------//
template <typename Archive>
void save(Archive &ar, const std::vector<bool> &t, const unsigned int)
{
    ar << unsigned(t.size());
    for (const bool elem : t)
    {
        ar << elem;
    }
}

template <typename Archive>
void load(Archive &ar, std::vector<bool> &t, const unsigned int)
{
    unsigned size(0);
    ar >> size;
    t.resize(size);
    for (size_t i = 0; i < size_t(size); i++)
    {
        bool elem;
        ar >> elem;
        t[i] = elem;
    }
}

//------------ a vector of any type --------------//
template<typename Archive, typename T>
void save(Archive &ar, const std::vector<T> &t, const unsigned int)
{
    ar << unsigned(t.size());
    for (const auto &elem : t)
    {
        ar << elem;
    }
}

template<typename Archive, typename T>
void load(Archive &ar, std::vector<T> &t, const unsigned int)
{
    unsigned size(0);
    ar >> size;
    t.resize(size);
    for (size_t i = 0; i < size_t(size); i++)
    {
        T elem;
        ar >> elem;
        t[i] = std::move(elem);
    }
}

template <typename Archive, typename T>
void serialize(Archive &ar, std::vector<T> &t, const unsigned int ver)
{
    Pothos::serialization::invokeSplit(ar, t, ver);
}

}}
