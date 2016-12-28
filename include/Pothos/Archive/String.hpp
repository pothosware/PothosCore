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
#include <Pothos/Archive/Numbers.hpp>
#include <Pothos/Archive/Macros.hpp>
#include <string>

namespace Pothos {
namespace serialization {

template<typename Archive>
void save(Archive &ar, const std::string &t, const unsigned int)
{
    ar << int(t.size());
    ar.writeBytes(t.data(), t.size());
}

template<typename Archive>
void load(Archive &ar, std::string &t, const unsigned int)
{
    int size(0);
    ar >> size;
    t.resize(size);
    ar.readBytes((void *)t.data(), t.size());
}

}}

//TODO wstring

POTHOS_SERIALIZATION_SPLIT_FREE(std::string)
POTHOS_CLASS_EXPORT(std::string)
