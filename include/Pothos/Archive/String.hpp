///
/// \file Archive/String.hpp
///
/// String support for serialization.
///
/// \copyright
/// Copyright (c) 2016 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Archive/Invoke.hpp>
#include <Pothos/Archive/Numbers.hpp>
#include <Pothos/Archive/BinaryObject.hpp>
#include <string>

namespace Pothos {
namespace serialization {

template<typename Archive, typename T, typename Traits, typename Allocator>
void save(Archive &ar, const std::basic_string<T, Traits, Allocator> &t, const unsigned int)
{
    ar << unsigned(t.size());
    BinaryObject bo(t.data(), t.size());
    ar << bo;
}

template<typename Archive, typename T, typename Traits, typename Allocator>
void load(Archive &ar, std::basic_string<T, Traits, Allocator> &t, const unsigned int)
{
    unsigned size(0);
    ar >> size;
    t.resize(size);
    BinaryObject bo(t.data(), t.size());
    ar >> bo;
}

template <typename Archive, typename T, typename Traits, typename Allocator>
void serialize(Archive &ar, std::basic_string<T, Traits, Allocator> &t, const unsigned int ver)
{
    Pothos::serialization::invokeSplit(ar, t, ver);
}

}}
