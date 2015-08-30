
#if !defined(POTHOS_PP_IS_ITERATING)

///// header body

#ifndef POTHOS_MPL_AUX778076_ADVANCE_BACKWARD_HPP_INCLUDED
#define POTHOS_MPL_AUX778076_ADVANCE_BACKWARD_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2000-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: advance_backward.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-10 23:19:02 -0700 (Fri, 10 Oct 2008) $
// $Revision: 49267 $

#if !defined(POTHOS_MPL_PREPROCESSING_MODE)
#   include <Pothos/serialization/impl/mpl/prior.hpp>
#   include <Pothos/serialization/impl/mpl/apply_wrap.hpp>
#endif

#include <Pothos/serialization/impl/mpl/aux_/config/use_preprocessed.hpp>

#if    !defined(POTHOS_MPL_CFG_NO_PREPROCESSED_HEADERS) \
    && !defined(POTHOS_MPL_PREPROCESSING_MODE)

#   define POTHOS_MPL_PREPROCESSED_HEADER advance_backward.hpp
#   include <Pothos/serialization/impl/mpl/aux_/include_preprocessed.hpp>

#else

#   include <Pothos/serialization/impl/mpl/limits/unrolling.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/nttp_decl.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/config/eti.hpp>

#   include <Pothos/serialization/impl/preprocessor/iterate.hpp>
#   include <Pothos/serialization/impl/preprocessor/cat.hpp>
#   include <Pothos/serialization/impl/preprocessor/inc.hpp>

namespace Pothos { namespace mpl { namespace aux {

// forward declaration
template< POTHOS_MPL_AUX_NTTP_DECL(long, N) > struct advance_backward;

#   define POTHOS_PP_ITERATION_PARAMS_1 \
    (3,(0, POTHOS_MPL_LIMIT_UNROLLING, <boost/mpl/aux_/advance_backward.hpp>))
#   include POTHOS_PP_ITERATE()

// implementation for N that exceeds BOOST_MPL_LIMIT_UNROLLING
template< POTHOS_MPL_AUX_NTTP_DECL(long, N) >
struct advance_backward
{
    template< typename Iterator > struct apply
    {
        typedef typename apply_wrap1<
              advance_backward<POTHOS_MPL_LIMIT_UNROLLING>
            , Iterator
            >::type chunk_result_;

        typedef typename apply_wrap1<
              advance_backward<(
                (N - POTHOS_MPL_LIMIT_UNROLLING) < 0
                    ? 0
                    : N - POTHOS_MPL_LIMIT_UNROLLING
                    )>
            , chunk_result_
            >::type type;
    };
};

}}}

#endif // BOOST_MPL_CFG_NO_PREPROCESSED_HEADERS
#endif // BOOST_MPL_AUX778076_ADVANCE_BACKWARD_HPP_INCLUDED

///// iteration, depth == 1

// For gcc 4.4 compatability, we must include the
// BOOST_PP_ITERATION_DEPTH test inside an #else clause.
#else // BOOST_PP_IS_ITERATING
#if POTHOS_PP_ITERATION_DEPTH() == 1
#define i_ POTHOS_PP_FRAME_ITERATION(1)

template<>
struct advance_backward< POTHOS_PP_FRAME_ITERATION(1) >
{
    template< typename Iterator > struct apply
    {
        typedef Iterator iter0;

#if i_ > 0
#   define POTHOS_PP_ITERATION_PARAMS_2 \
    (3,(1, POTHOS_PP_FRAME_ITERATION(1), <boost/mpl/aux_/advance_backward.hpp>))
#   include POTHOS_PP_ITERATE()
#endif

        typedef POTHOS_PP_CAT(iter,POTHOS_PP_FRAME_ITERATION(1)) type;
    };

#if defined(POTHOS_MPL_CFG_MSVC_60_ETI_BUG)
    /// ETI workaround
    template<> struct apply<int>
    {
        typedef int type;
    };
#endif
};

#undef i_

///// iteration, depth == 2

#elif POTHOS_PP_ITERATION_DEPTH() == 2

#   define AUX778076_ITER_0 POTHOS_PP_CAT(iter,POTHOS_PP_DEC(POTHOS_PP_FRAME_ITERATION(2)))
#   define AUX778076_ITER_1 POTHOS_PP_CAT(iter,POTHOS_PP_FRAME_ITERATION(2))

        typedef typename prior<AUX778076_ITER_0>::type AUX778076_ITER_1;
        
#   undef AUX778076_ITER_1
#   undef AUX778076_ITER_0

#endif // BOOST_PP_ITERATION_DEPTH()
#endif // BOOST_PP_IS_ITERATING
