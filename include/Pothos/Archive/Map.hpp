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
#include <Pothos/Archive/Archive.hpp>
#include <Pothos/Archive/Numbers.hpp>
#include <type_traits>
#include <map>

namespace Pothos {
namespace serialization {

template<typename Archive, typename K, typename T>
void save(Archive &ar, const std::map<K, T> &t, const unsigned int)
{
    ar << unsigned(t.size());
    for (const auto &pair : t)
    {
        ar << pair.first;
        ar << pair.second;
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
        K key; ar >> key;
        T value; ar >> value;
        t.emplace(key, value);
    }
}

template <typename Archive, typename K, typename T>
void serialize(Archive &ar, std::map<K, T> &t, const unsigned int ver)
{
    Pothos::serialization::invoke_load_save(ar, t, ver);
}

}}
