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

/***********************************************************************
 * Integer sequence implementation for C++11
 * This can be removed when C++14 is required.
 * http://en.cppreference.com/w/cpp/utility/integer_sequence
 **********************************************************************/
//! \cond

template<typename T, T... Ints>
struct integer_sequence
{
    typedef T value_type;

    static constexpr std::size_t size(void) noexcept
    {
        return sizeof...(Ints);
    }
};

template<typename T, int N, T... Ints>
struct GenSeq : GenSeq<T, N-1, T(N-1), Ints...> {};

template<typename T, T... Ints>
struct GenSeq<T, 0, Ints...>
{
    typedef integer_sequence<T, Ints...> Type;
};

template<std::size_t... Ints>
using index_sequence = integer_sequence<std::size_t, Ints...>;

template<typename T, T N>
using make_integer_sequence = typename GenSeq<T, int(N)>::Type;

template<std::size_t N>
using make_index_sequence = make_integer_sequence<std::size_t, N>;

template<typename... T>
using index_sequence_for = make_index_sequence<sizeof...(T)>;

//! \endcond

} //namespace Util
} //namespace Pothos
