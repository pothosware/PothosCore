///
/// \file Archive/Map.hpp
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
#include <vector>
#include <utility> //move
#include <map>

namespace Pothos {
namespace serialization {

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
    Pothos::serialization::invokeLoadSave(ar, t, ver);
}

}}
