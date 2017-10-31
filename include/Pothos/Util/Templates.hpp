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

/*!*********************************************************************
 * \defgroup unwrapref Implementation for unwrap_refwrapper and special_decay_t
 * special_decay_t = decay + unwrapping a reference wrapper
 * \see http://en.cppreference.com/w/cpp/utility/tuple/make_tuple
 * \{
 **********************************************************************/

/*!
 * Template base implementation to extract the type of non-reference wrappers
 * \tparam T the input type to the template
 */
template <typename T>
struct unwrap_refwrapper
{
    using type = T; //!< the input type of non-reference wrappers
};

/*!
 * Template specialization to extract the type held within a reference wrapper
 * \tparam T the input type to the template
 */
template <typename T>
struct unwrap_refwrapper<std::reference_wrapper<T>>
{
    using type = T&; //!< the type held within a reference wrapper
};

/*!
 * Strip const and reference from a type, and remove optional reference wrapper.
 * \tparam T the input type to the template
 */
template <typename T>
using special_decay_t = typename unwrap_refwrapper<typename std::decay<T>::type>::type;
//! \}

/*!*********************************************************************
 * \defgroup intseq Integer sequence implementation for C++11
 * This can be removed when C++14 is required.
 * \see http://en.cppreference.com/w/cpp/utility/integer_sequence
 * \{
 **********************************************************************/

/*!
 * A parameter pack of incrementing numbers 0 to N-1
 * \tparam T the integer type
 * \tparam Ints the parameter pack of integers
 */
template<typename T, T... Ints>
struct integer_sequence
{
    typedef T value_type; //!< The type of each integer

    //! The size of the integer pack
    static constexpr std::size_t size(void) noexcept
    {
        return sizeof...(Ints);
    }
};

//! \cond
template<typename T, int N, T... Ints>
struct GenSeq : GenSeq<T, N-1, T(N-1), Ints...> {};

template<typename T, T... Ints>
struct GenSeq<T, 0, Ints...>
{
    typedef integer_sequence<T, Ints...> Type;
};
//! \endcond

/*!
 * Specialize the integer sequence for size_t
 * \tparam Ints the parameter pack of integers
 */
template<std::size_t... Ints>
using index_sequence = integer_sequence<std::size_t, Ints...>;

/*!
 * Make an integer sequence from the length
 * \tparam T the integer type
 * \tparam N the length
 */
template<typename T, T N>
using make_integer_sequence = typename GenSeq<T, int(N)>::Type;

/*!
 * Make an index sequence from the length
 * \tparam N the length
 */
template<std::size_t N>
using make_index_sequence = make_integer_sequence<std::size_t, N>;

/*!
 * Make an index sequence from a parameter pack
 * \tparam T the parameter pack
 */
template<typename... T>
using index_sequence_for = make_index_sequence<sizeof...(T)>;
//! \}

/*!*********************************************************************
 * Disable SFINAE type used with universal reference constructors
 * \see http://ericniebler.com/2013/08/07/universal-references-and-the-copy-constructo/
 **********************************************************************/
template<typename BaseType, typename OtherType>
using disable_if_same = typename std::enable_if<
    not std::is_same<BaseType, typename std::decay<OtherType>::type>::value>::type;

} //namespace Util
} //namespace Pothos
