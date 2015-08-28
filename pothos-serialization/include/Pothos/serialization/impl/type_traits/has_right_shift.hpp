//  (C) Copyright 2009-2011 Frederic Bron.
//
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).
//
//  See http://www.boost.org/libs/type_traits for most recent version including documentation.

#ifndef POTHOS_TT_HAS_RIGHT_SHIFT_HPP_INCLUDED
#define POTHOS_TT_HAS_RIGHT_SHIFT_HPP_INCLUDED

#define POTHOS_TT_TRAIT_NAME has_right_shift
#define POTHOS_TT_TRAIT_OP >>
#define POTHOS_TT_FORBIDDEN_IF\
   ::Pothos::type_traits::ice_or<\
      /* Lhs==fundamental and Rhs==fundamental and (Lhs!=integral or Rhs!=integral) */\
      ::Pothos::type_traits::ice_and<\
         ::Pothos::is_fundamental< Lhs_nocv >::value,\
         ::Pothos::is_fundamental< Rhs_nocv >::value,\
         ::Pothos::type_traits::ice_or<\
            ::Pothos::type_traits::ice_not< ::Pothos::is_integral< Lhs_noref >::value >::value,\
            ::Pothos::type_traits::ice_not< ::Pothos::is_integral< Rhs_noref >::value >::value\
         >::value\
      >::value,\
      /* Lhs==fundamental and Rhs==pointer */\
      ::Pothos::type_traits::ice_and<\
         ::Pothos::is_fundamental< Lhs_nocv >::value,\
         ::Pothos::is_pointer< Rhs_noref >::value\
      >::value,\
      /* Rhs==fundamental and Lhs==pointer */\
      ::Pothos::type_traits::ice_and<\
         ::Pothos::is_fundamental< Rhs_nocv >::value,\
         ::Pothos::is_pointer< Lhs_noref >::value\
      >::value,\
      /* Lhs==pointer and Rhs==pointer */\
      ::Pothos::type_traits::ice_and<\
         ::Pothos::is_pointer< Lhs_noref >::value,\
         ::Pothos::is_pointer< Rhs_noref >::value\
      >::value\
   >::value


#include <Pothos/serialization/impl/type_traits/detail/has_binary_operator.hpp>

#undef POTHOS_TT_TRAIT_NAME
#undef POTHOS_TT_TRAIT_OP
#undef POTHOS_TT_FORBIDDEN_IF

#endif
