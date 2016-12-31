///
/// \file Archive/Map.hpp
///
/// Map support for serialization.
///
/// \copyright
/// Copyright (c) 2016 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Archive/Invoke.hpp>
#include <Pothos/Archive/Numbers.hpp>
#include <Pothos/Archive/Pair.hpp>
#include <utility> //move
#include <map>

namespace Pothos {
namespace serialization {

template<typename Archive, typename K, typename T>
void save(Archive &ar, const std::map<K, T> &t, const unsigned int)
{
    ar << unsigned(t.size());
    for (const auto &pair : t)
    {
        ar << pair;
    }
}

template<typename Archive, typename K, typename T>
void load(Archive &ar, std::map<K, T> &t, const unsigned int)
{
    t.clear();
    unsigned size(0);
    ar >> size;
    for (size_t i = 0; i < size_t(size); i++)
    {
        std::pair<K, T> pair;
        ar >> pair;
        t.emplace(std::move(pair));
    }
}

template <typename Archive, typename K, typename T>
void serialize(Archive &ar, std::map<K, T> &t, const unsigned int ver)
{
    Pothos::serialization::invokeSplit(ar, t, ver);
}

}}
