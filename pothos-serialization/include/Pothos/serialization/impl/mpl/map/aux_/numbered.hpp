
// NO INCLUDE GUARDS, THE HEADER IS INTENDED FOR MULTIPLE INCLUSION

#if !defined(POTHOS_PP_IS_ITERATING)

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

#else

#include <Pothos/serialization/impl/mpl/aux_/config/typeof.hpp>
#include <Pothos/serialization/impl/mpl/aux_/config/ctps.hpp>
#include <Pothos/serialization/impl/preprocessor/enum_params.hpp>
#include <Pothos/serialization/impl/preprocessor/dec.hpp>
#include <Pothos/serialization/impl/preprocessor/cat.hpp>

#define i_ POTHOS_PP_FRAME_ITERATION(1)

#   define AUX778076_MAP_TAIL(map, i_, P) \
    POTHOS_PP_CAT(map,i_)< \
          POTHOS_PP_ENUM_PARAMS(i_, P) \
        > \
    /**/


#if defined(POTHOS_MPL_CFG_TYPEOF_BASED_SEQUENCES)

template<
      POTHOS_PP_ENUM_PARAMS(i_, typename P)
    >
struct POTHOS_PP_CAT(map,i_)
    : m_item<
          typename POTHOS_PP_CAT(P,POTHOS_PP_DEC(i_))::first
        , typename POTHOS_PP_CAT(P,POTHOS_PP_DEC(i_))::second
        , AUX778076_MAP_TAIL(map,POTHOS_PP_DEC(i_),P)
        >
{
    typedef POTHOS_PP_CAT(map,i_) type;
};

#else // "brute force" implementation

#   if !defined(POTHOS_NO_TEMPLATE_PARTIAL_SPECIALIZATION)

template< typename Map>
struct m_at<Map,POTHOS_PP_DEC(i_)>
{
    typedef typename Map::POTHOS_PP_CAT(item,POTHOS_PP_DEC(i_)) type;
};

template< typename Key, typename T, typename Base >
struct m_item<i_,Key,T,Base>
    : m_item_<Key,T,Base>
{
    typedef pair<Key,T> POTHOS_PP_CAT(item,POTHOS_PP_DEC(i_));
};

#   else

template<>
struct m_at_impl<POTHOS_PP_DEC(i_)>
{
    template< typename Map > struct result_
    {
        typedef typename Map::POTHOS_PP_CAT(item,POTHOS_PP_DEC(i_)) type;
    };
};

template<>
struct m_item_impl<i_>
{
    template< typename Key, typename T, typename Base > struct result_
        : m_item_<Key,T,Base>
    {
        typedef pair<Key,T> POTHOS_PP_CAT(item,POTHOS_PP_DEC(i_));
    };
};

#   endif

template<
      POTHOS_PP_ENUM_PARAMS(i_, typename P)
    >
struct POTHOS_PP_CAT(map,i_)
    : m_item<
          i_
        , typename POTHOS_PP_CAT(P,POTHOS_PP_DEC(i_))::first
        , typename POTHOS_PP_CAT(P,POTHOS_PP_DEC(i_))::second
        , AUX778076_MAP_TAIL(map,POTHOS_PP_DEC(i_),P)
        >
{
    typedef POTHOS_PP_CAT(map,i_) type;
};

#endif // BOOST_MPL_CFG_TYPEOF_BASED_SEQUENCES

#   undef AUX778076_MAP_TAIL

#undef i_

#endif // BOOST_PP_IS_ITERATING
