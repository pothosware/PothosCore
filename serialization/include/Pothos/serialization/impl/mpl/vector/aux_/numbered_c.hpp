
// NO INCLUDE GUARDS, THE HEADER IS INTENDED FOR MULTIPLE INCLUSION

#if defined(POTHOS_PP_IS_ITERATING)

// Copyright Aleksey Gurtovoy 2000-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: numbered_c.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-10 23:19:02 -0700 (Fri, 10 Oct 2008) $
// $Revision: 49267 $

#include <Pothos/serialization/impl/preprocessor/enum_params.hpp>
#include <Pothos/serialization/impl/preprocessor/enum_shifted_params.hpp>
#include <Pothos/serialization/impl/preprocessor/comma_if.hpp>
#include <Pothos/serialization/impl/preprocessor/repeat.hpp>
#include <Pothos/serialization/impl/preprocessor/dec.hpp>
#include <Pothos/serialization/impl/preprocessor/cat.hpp>

#define i_ POTHOS_PP_FRAME_ITERATION(1)

#if defined(POTHOS_MPL_CFG_TYPEOF_BASED_SEQUENCES)

#   define AUX778076_VECTOR_TAIL(vector, i_, C) \
    POTHOS_PP_CAT(POTHOS_PP_CAT(vector,i_),_c)<T \
          POTHOS_PP_COMMA_IF(i_) POTHOS_PP_ENUM_PARAMS(i_, C) \
        > \
    /**/

#if i_ > 0
template<
      typename T
    , POTHOS_PP_ENUM_PARAMS(i_, T C)
    >
struct POTHOS_PP_CAT(POTHOS_PP_CAT(vector,i_),_c)
    : v_item<
          integral_c<T,POTHOS_PP_CAT(C,POTHOS_PP_DEC(i_))>
        , AUX778076_VECTOR_TAIL(vector,POTHOS_PP_DEC(i_),C)
        >
{
    typedef POTHOS_PP_CAT(POTHOS_PP_CAT(vector,i_),_c) type;
    typedef T value_type;
};
#endif

#   undef AUX778076_VECTOR_TAIL

#else // "brute force" implementation

#   define AUX778076_VECTOR_C_PARAM_FUNC(unused, i_, param) \
    POTHOS_PP_COMMA_IF(i_) \
    integral_c<T,POTHOS_PP_CAT(param,i_)> \
    /**/

template<
      typename T
    , POTHOS_PP_ENUM_PARAMS(i_, T C)
    >
struct POTHOS_PP_CAT(POTHOS_PP_CAT(vector,i_),_c)
    : POTHOS_PP_CAT(vector,i_)< POTHOS_PP_REPEAT(i_,AUX778076_VECTOR_C_PARAM_FUNC,C) >
{
    typedef POTHOS_PP_CAT(POTHOS_PP_CAT(vector,i_),_c) type;
    typedef T value_type;
};

#   undef AUX778076_VECTOR_C_PARAM_FUNC

#endif // BOOST_MPL_CFG_TYPEOF_BASED_SEQUENCES

#undef i_

#endif // BOOST_PP_IS_ITERATING
