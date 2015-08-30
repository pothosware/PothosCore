
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

#include <Pothos/serialization/impl/preprocessor/repetition/enum_params.hpp>
#include <Pothos/serialization/impl/preprocessor/repetition/enum_trailing_params.hpp>
#include <Pothos/serialization/impl/preprocessor/dec.hpp>
#include <Pothos/serialization/impl/preprocessor/cat.hpp>

#define i_ POTHOS_PP_FRAME_ITERATION(1)

#   define AUX778076_SET_C_TAIL(set, i_, T, C) \
    POTHOS_PP_CAT(POTHOS_PP_CAT(set,i_),_c)< \
          T POTHOS_PP_ENUM_TRAILING_PARAMS(i_, C) \
        > \
    /**/

template<
      typename T
    , POTHOS_PP_ENUM_PARAMS(i_, T C)
    >
struct POTHOS_PP_CAT(POTHOS_PP_CAT(set,i_),_c)
    : s_item<
          integral_c<T,POTHOS_PP_CAT(C,POTHOS_PP_DEC(i_))>
        , AUX778076_SET_C_TAIL(set,POTHOS_PP_DEC(i_), T, C)
        >
{
    typedef POTHOS_PP_CAT(POTHOS_PP_CAT(set,i_),_c) type;
};

#   undef AUX778076_SET_C_TAIL

#undef i_

#endif // BOOST_PP_IS_ITERATING
