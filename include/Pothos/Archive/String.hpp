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
#include <Pothos/Archive/Macros.hpp>
#include <string>

namespace Pothos {
namespace serialization {

template<class Archive>
void save(Archive &a, const std::string &t, const unsigned int)
{
    
}

template<class Archive>
void load(Archive &a, std::string &t, const unsigned int)
{
    
}

}}

POTHOS_SERIALIZATION_SPLIT_FREE(std::string)
