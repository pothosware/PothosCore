///
/// \file Archive/Numbers.hpp
///
/// Numeric support for serialization.
/// Integer and floating point types.
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
void save(Archive &a, const int &t, const unsigned int)
{
    
}

template<class Archive>
void load(Archive &a, int &t, const unsigned int)
{
    
}

}}

POTHOS_SERIALIZATION_SPLIT_FREE(int)
