//
//! Copyright (c) 2011
//! Brandon Kohn
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef POTHOS_NUMERIC_CAST_TRAITS_HPP
#define POTHOS_NUMERIC_CAST_TRAITS_HPP

#include <Pothos/serialization/impl/numeric/conversion/converter_policies.hpp>

namespace Pothos { namespace numeric {

    template <typename Target, typename Source, typename EnableIf = void>
    struct numeric_cast_traits
    {
        typedef def_overflow_handler    overflow_policy;
        typedef UseInternalRangeChecker range_checking_policy;
        typedef Trunc<Source>           rounding_policy;
    };

}}//namespace boost::numeric;

#if !defined( POTHOS_NUMERIC_CONVERSION_RELAX_BUILT_IN_CAST_TRAITS )
#include <Pothos/serialization/impl/cstdint.hpp>
#include <Pothos/serialization/impl/numeric/conversion/detail/numeric_cast_traits.hpp>
#endif//!defined BOOST_NUMERIC_CONVERSION_RELAX_BUILT_IN_CAST_TRAITS

#endif//BOOST_NUMERIC_CAST_TRAITS_HPP
