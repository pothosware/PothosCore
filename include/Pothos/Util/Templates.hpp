///
/// \file Util/Templates.hpp
///
/// Utility templates and metaprograming classes.
///
/// \copyright
/// Copyright (c) 2017-2017 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Config.hpp>
#include <type_traits> //std::decay
#include <functional> //std::reference_wrapper

namespace Pothos {
namespace Util {

/***********************************************************************
 * special_decay_t = decay + unwrapping a reference wrapper
 * http://en.cppreference.com/w/cpp/utility/tuple/make_tuple
 **********************************************************************/
template <typename T>
struct unwrap_refwrapper
{
    using type = T;
};

template <typename T>
struct unwrap_refwrapper<std::reference_wrapper<T>>
{
    using type = T&;
};

template <typename T>
using special_decay_t = typename unwrap_refwrapper<typename std::decay<T>::type>::type;

} //namespace Util
} //namespace Pothos
