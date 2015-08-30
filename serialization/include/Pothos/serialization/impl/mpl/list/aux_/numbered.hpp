
// NO INCLUDE GUARDS, THE HEADER IS INTENDED FOR MULTIPLE INCLUSION

// Copyright Peter Dimov 2000-2002
// Copyright Aleksey Gurtovoy 2000-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: numbered.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-10 23:19:02 -0700 (Fri, 10 Oct 2008) $
// $Revision: 49267 $

#if defined(POTHOS_PP_IS_ITERATING)

#include <Pothos/serialization/impl/preprocessor/enum_params.hpp>
#include <Pothos/serialization/impl/preprocessor/enum_shifted_params.hpp>
#include <Pothos/serialization/impl/preprocessor/dec.hpp>
#include <Pothos/serialization/impl/preprocessor/cat.hpp>

#define i POTHOS_PP_FRAME_ITERATION(1)

#if i == 1

template<
      POTHOS_PP_ENUM_PARAMS(i, typename T)
    >
struct list1
    : l_item<
          long_<1>
        , T0
        , l_end
        >
{
    typedef list1 type;
};

#else

#   define MPL_AUX_LIST_TAIL(list, i, T) \
    POTHOS_PP_CAT(list,POTHOS_PP_DEC(i))< \
      POTHOS_PP_ENUM_SHIFTED_PARAMS(i, T) \
    > \
    /**/
    
template<
      POTHOS_PP_ENUM_PARAMS(i, typename T)
    >
struct POTHOS_PP_CAT(list,i)
    : l_item<
          long_<i>
        , T0
        , MPL_AUX_LIST_TAIL(list,i,T)
        >
{
    typedef POTHOS_PP_CAT(list,i) type;
};

#   undef MPL_AUX_LIST_TAIL

#endif // i == 1

#undef i

#endif // BOOST_PP_IS_ITERATING
