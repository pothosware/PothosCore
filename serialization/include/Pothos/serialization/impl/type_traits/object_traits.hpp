//  (C) Copyright Steve Cleary, Beman Dawes, Howard Hinnant & John Maddock 2000.
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).
//
//  See http://www.boost.org/libs/type_traits for most recent version including documentation.
//
//  defines object traits classes:
//  is_object, is_scalar, is_class, is_compound, is_pod, 
//  has_trivial_constructor, has_trivial_copy, has_trivial_assign, 
//  has_trivial_destructor, is_empty.
//

#ifndef POTHOS_TT_OBJECT_TRAITS_HPP_INLCUDED
#define POTHOS_TT_OBJECT_TRAITS_HPP_INLCUDED

#include <Pothos/serialization/impl/type_traits/has_trivial_assign.hpp>
#include <Pothos/serialization/impl/type_traits/has_trivial_constructor.hpp>
#include <Pothos/serialization/impl/type_traits/has_trivial_copy.hpp>
#include <Pothos/serialization/impl/type_traits/has_trivial_destructor.hpp>
#include <Pothos/serialization/impl/type_traits/has_nothrow_constructor.hpp>
#include <Pothos/serialization/impl/type_traits/has_nothrow_copy.hpp>
#include <Pothos/serialization/impl/type_traits/has_nothrow_assign.hpp>
#include <Pothos/serialization/impl/type_traits/is_base_and_derived.hpp>
#include <Pothos/serialization/impl/type_traits/is_class.hpp>
#include <Pothos/serialization/impl/type_traits/is_compound.hpp>
#include <Pothos/serialization/impl/type_traits/is_empty.hpp>
#include <Pothos/serialization/impl/type_traits/is_object.hpp>
#include <Pothos/serialization/impl/type_traits/is_pod.hpp>
#include <Pothos/serialization/impl/type_traits/is_scalar.hpp>
#include <Pothos/serialization/impl/type_traits/is_stateless.hpp>

#endif // BOOST_TT_OBJECT_TRAITS_HPP_INLCUDED
