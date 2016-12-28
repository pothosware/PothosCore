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
#include <Pothos/Archive/Archive.hpp>
#include <Pothos/Archive/Numbers.hpp>
#include <string>

namespace Pothos {
namespace serialization {

template<typename Archive, typename T>
void save(Archive &ar, const std::basic_string<T> &t, const unsigned int)
{
    ar << unsigned(t.size());
    ar.writeBytes(t.data(), t.size());
}

template<typename Archive, typename T>
void load(Archive &ar, std::basic_string<T> &t, const unsigned int)
{
    unsigned size(0);
    ar >> size;
    t.resize(size);
    ar.readBytes((void *)t.data(), t.size());
}

template <typename Archive, typename T>
void serialize(Archive &ar, std::basic_string<T> &t, const unsigned int ver)
{
    Pothos::serialization::invoke_load_save(ar, t, ver);
}

}}
