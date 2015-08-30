
//  (C) Copyright Rani Sharoni 2003-2005.
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).
//
//  See http://www.boost.org/libs/type_traits for most recent version including documentation.
 
#ifndef POTHOS_TT_IS_BASE_OF_HPP_INCLUDED
#define POTHOS_TT_IS_BASE_OF_HPP_INCLUDED

#include <Pothos/serialization/impl/type_traits/is_base_and_derived.hpp>
#include <Pothos/serialization/impl/type_traits/is_same.hpp>
#include <Pothos/serialization/impl/type_traits/is_class.hpp>
#include <Pothos/serialization/impl/type_traits/detail/ice_or.hpp>
#include <Pothos/serialization/impl/type_traits/detail/ice_and.hpp>

// should be the last #include
#include <Pothos/serialization/impl/type_traits/detail/bool_trait_def.hpp>

namespace Pothos {

   namespace detail{
      template <class B, class D>
      struct is_base_of_imp
      {
          typedef typename remove_cv<B>::type ncvB;
          typedef typename remove_cv<D>::type ncvD;
          POTHOS_STATIC_CONSTANT(bool, value = (::Pothos::type_traits::ice_or<      
            (::Pothos::detail::is_base_and_derived_impl<ncvB,ncvD>::value),
            (::Pothos::type_traits::ice_and< ::Pothos::is_same<ncvB,ncvD>::value, ::Pothos::is_class<ncvB>::value>::value)>::value));
      };
   }

POTHOS_TT_AUX_BOOL_TRAIT_DEF2(
      is_base_of
    , Base
    , Derived
    , (::Pothos::detail::is_base_of_imp<Base, Derived>::value))

#ifndef POTHOS_NO_TEMPLATE_PARTIAL_SPECIALIZATION
POTHOS_TT_AUX_BOOL_TRAIT_PARTIAL_SPEC2_2(typename Base,typename Derived,is_base_of,Base&,Derived,false)
POTHOS_TT_AUX_BOOL_TRAIT_PARTIAL_SPEC2_2(typename Base,typename Derived,is_base_of,Base,Derived&,false)
POTHOS_TT_AUX_BOOL_TRAIT_PARTIAL_SPEC2_2(typename Base,typename Derived,is_base_of,Base&,Derived&,false)
#endif

} // namespace boost

#include <Pothos/serialization/impl/type_traits/detail/bool_trait_undef.hpp>

#endif // BOOST_TT_IS_BASE_AND_DERIVED_HPP_INCLUDED
