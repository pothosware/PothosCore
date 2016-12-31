///
/// \file Archive/Set.hpp
///
/// Set support for serialization.
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
#include <set>

namespace Pothos {
namespace serialization {

template<typename Archive, typename T, typename Compare, typename Allocator>
void save(Archive &ar, const std::set<T, Compare, Allocator> &t, const unsigned int)
{
    ar << unsigned(t.size());
    for (const auto &elem : t)
    {
        ar << elem;
    }
}

template<typename Archive, typename T, typename Compare, typename Allocator>
void load(Archive &ar, std::set<T, Compare, Allocator> &t, const unsigned int)
{
    t.clear();
    unsigned size(0);
    ar >> size;
    for (size_t i = 0; i < size_t(size); i++)
    {
        T elem;
        ar >> elem;
        t.insert(std::move(elem));
    }
}

template <typename Archive, typename T, typename Compare, typename Allocator>
void serialize(Archive &ar, std::set<T, Compare, Allocator> &t, const unsigned int ver)
{
    Pothos::serialization::invokeSplit(ar, t, ver);
}

}}
