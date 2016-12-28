///
/// \file Archive/Pair.hpp
///
/// Pair support for serialization.
///
/// \copyright
/// Copyright (c) 2016 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Config.hpp>
#include <utility> //pair

namespace Pothos {
namespace serialization {

template <typename Archive, typename F, typename L>
void serialize(Archive &ar, std::pair<F, L> &t, const unsigned int)
{
    ar & t.first;
    ar & t.second;
}

}}
