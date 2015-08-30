
// NO INCLUDE GUARDS, THE HEADER IS INTENDED FOR MULTIPLE INCLUSION

#if !defined(POTHOS_PP_IS_ITERATING)

// Copyright Aleksey Gurtovoy 2000-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: fold_impl_body.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-10 23:19:02 -0700 (Fri, 10 Oct 2008) $
// $Revision: 49267 $

#   include <Pothos/serialization/impl/mpl/limits/unrolling.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/preprocessor/repeat.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/config/workaround.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/config/ctps.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/nttp_decl.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/config/eti.hpp>

#   include <Pothos/serialization/impl/preprocessor/iterate.hpp>
#   include <Pothos/serialization/impl/preprocessor/dec.hpp>
#   include <Pothos/serialization/impl/preprocessor/cat.hpp>

// local macros, #undef-ined at the end of the header

#   define AUX778076_ITER_FOLD_STEP(unused, i, unused2) \
    typedef typename apply2< \
          ForwardOp \
        , POTHOS_PP_CAT(state,i) \
        , AUX778076_FOLD_IMPL_OP(POTHOS_PP_CAT(iter,i)) \
        >::type POTHOS_PP_CAT(state,POTHOS_PP_INC(i)); \
    typedef typename mpl::next<POTHOS_PP_CAT(iter,i)>::type \
        POTHOS_PP_CAT(iter,POTHOS_PP_INC(i)); \
    /**/

#   define AUX778076_FOLD_IMPL_NAME \
    POTHOS_PP_CAT(AUX778076_FOLD_IMPL_NAME_PREFIX,_impl) \
    /**/

#   define AUX778076_FOLD_CHUNK_NAME \
    POTHOS_PP_CAT(AUX778076_FOLD_IMPL_NAME_PREFIX,_chunk) \
    /**/

namespace Pothos { namespace mpl { namespace aux {

/// forward declaration
template<
      POTHOS_MPL_AUX_NTTP_DECL(int, N)
    , typename First
    , typename Last
    , typename State
    , typename ForwardOp
    > 
struct AUX778076_FOLD_IMPL_NAME;

#if !defined(POTHOS_NO_TEMPLATE_PARTIAL_SPECIALIZATION)

#   if !POTHOS_WORKAROUND(__BORLANDC__, < 0x600)

#   define POTHOS_PP_ITERATION_PARAMS_1 \
    (3,(0, POTHOS_MPL_LIMIT_UNROLLING, <boost/mpl/aux_/fold_impl_body.hpp>))
#   include POTHOS_PP_ITERATE()

// implementation for N that exceeds BOOST_MPL_LIMIT_UNROLLING
template<
      POTHOS_MPL_AUX_NTTP_DECL(int, N)
    , typename First
    , typename Last
    , typename State
    , typename ForwardOp
    > 
struct AUX778076_FOLD_IMPL_NAME
{
    typedef AUX778076_FOLD_IMPL_NAME<
          POTHOS_MPL_LIMIT_UNROLLING
        , First
        , Last
        , State
        , ForwardOp
        > chunk_;

    typedef AUX778076_FOLD_IMPL_NAME<
          ( (N - POTHOS_MPL_LIMIT_UNROLLING) < 0 ? 0 : N - POTHOS_MPL_LIMIT_UNROLLING )
        , typename chunk_::iterator
        , Last
        , typename chunk_::state
        , ForwardOp
        > res_;
        
    typedef typename res_::state state;
    typedef typename res_::iterator iterator;
};

// fallback implementation for sequences of unknown size
template<
      typename First
    , typename Last
    , typename State
    , typename ForwardOp
    > 
struct AUX778076_FOLD_IMPL_NAME<-1,First,Last,State,ForwardOp>
    : AUX778076_FOLD_IMPL_NAME<
          -1
        , typename mpl::next<First>::type
        , Last
        , typename apply2<ForwardOp,State,AUX778076_FOLD_IMPL_OP(First)>::type
        , ForwardOp
        >
{
};

template<
      typename Last
    , typename State
    , typename ForwardOp
    > 
struct AUX778076_FOLD_IMPL_NAME<-1,Last,Last,State,ForwardOp>
{
    typedef State state;
    typedef Last iterator;
};

#   else // BOOST_WORKAROUND(__BORLANDC__, < 0x600)

// Borland have some serious problems with the unrolled version, so
// we always use a basic implementation
template<
      POTHOS_MPL_AUX_NTTP_DECL(int, N)
    , typename First
    , typename Last
    , typename State
    , typename ForwardOp
    > 
struct AUX778076_FOLD_IMPL_NAME
{
    typedef AUX778076_FOLD_IMPL_NAME<
          -1
        , typename mpl::next<First>::type
        , Last
        , typename apply2<ForwardOp,State,AUX778076_FOLD_IMPL_OP(First)>::type
        , ForwardOp
        > res_;

    typedef typename res_::state state;
    typedef typename res_::iterator iterator;
    typedef state type;
};

template<
      POTHOS_MPL_AUX_NTTP_DECL(int, N)
     , typename Last
    , typename State
    , typename ForwardOp
    > 
struct AUX778076_FOLD_IMPL_NAME<N,Last,Last,State,ForwardOp >
{
    typedef State state;
    typedef Last iterator;
    typedef state type;
};

#   endif // BOOST_WORKAROUND(__BORLANDC__, < 0x600)
 
#else // BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION

template< POTHOS_MPL_AUX_NTTP_DECL(int, N) >
struct AUX778076_FOLD_CHUNK_NAME;

#   define POTHOS_PP_ITERATION_PARAMS_1 \
    (3,(0, POTHOS_MPL_LIMIT_UNROLLING, <boost/mpl/aux_/fold_impl_body.hpp>))
#   include POTHOS_PP_ITERATE()

// implementation for N that exceeds BOOST_MPL_LIMIT_UNROLLING
template< POTHOS_MPL_AUX_NTTP_DECL(int, N) > 
struct AUX778076_FOLD_CHUNK_NAME
{
    template<
          typename First
        , typename Last
        , typename State
        , typename ForwardOp
        > 
    struct result_
    {
        typedef AUX778076_FOLD_IMPL_NAME<
              POTHOS_MPL_LIMIT_UNROLLING
            , First
            , Last
            , State
            , ForwardOp
            > chunk_;

        typedef AUX778076_FOLD_IMPL_NAME<
              ( (N - POTHOS_MPL_LIMIT_UNROLLING) < 0 ? 0 : N - POTHOS_MPL_LIMIT_UNROLLING )
            , typename chunk_::iterator
            , Last
            , typename chunk_::state
            , ForwardOp
            > res_;

        typedef typename res_::state state;
        typedef typename res_::iterator iterator;
    };
};

// fallback implementation for sequences of unknown size
template<
      typename First
    , typename Last
    , typename State
    , typename ForwardOp
    > 
struct POTHOS_PP_CAT(AUX778076_FOLD_IMPL_NAME_PREFIX,_step);

template<
      typename Last
    , typename State
    >
struct POTHOS_PP_CAT(AUX778076_FOLD_IMPL_NAME_PREFIX,_null_step)
{
    typedef Last iterator;
    typedef State state;
};

template<> 
struct AUX778076_FOLD_CHUNK_NAME<-1>
{
    template<
          typename First
        , typename Last
        , typename State
        , typename ForwardOp
        > 
    struct result_
    {
        typedef typename if_<
              typename is_same<First,Last>::type
            , POTHOS_PP_CAT(AUX778076_FOLD_IMPL_NAME_PREFIX,_null_step)<Last,State>
            , POTHOS_PP_CAT(AUX778076_FOLD_IMPL_NAME_PREFIX,_step)<First,Last,State,ForwardOp>
            >::type res_;

        typedef typename res_::state state;
        typedef typename res_::iterator iterator;
    };

#if defined(POTHOS_MPL_CFG_MSVC_60_ETI_BUG)
    /// ETI workaround
    template<> struct result_<int,int,int,int>
    {
        typedef int state;
        typedef int iterator;
    };
#endif
};

template<
      typename First
    , typename Last
    , typename State
    , typename ForwardOp
    > 
struct POTHOS_PP_CAT(AUX778076_FOLD_IMPL_NAME_PREFIX,_step)
{
    // can't inherit here - it breaks MSVC 7.0
    typedef AUX778076_FOLD_CHUNK_NAME<-1>::template result_<
          typename mpl::next<First>::type
        , Last
        , typename apply2<ForwardOp,State,AUX778076_FOLD_IMPL_OP(First)>::type
        , ForwardOp
        > chunk_;

    typedef typename chunk_::state state;
    typedef typename chunk_::iterator iterator;
};

template<
      POTHOS_MPL_AUX_NTTP_DECL(int, N)
    , typename First
    , typename Last
    , typename State
    , typename ForwardOp
    > 
struct AUX778076_FOLD_IMPL_NAME
    : AUX778076_FOLD_CHUNK_NAME<N>
        ::template result_<First,Last,State,ForwardOp>
{
};

#endif // BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION

}}}

#   undef AUX778076_FOLD_IMPL_NAME
#   undef AUX778076_FOLD_CHUNK_NAME
#   undef AUX778076_ITER_FOLD_STEP

#undef AUX778076_FOLD_IMPL_OP
#undef AUX778076_FOLD_IMPL_NAME_PREFIX

///// iteration

#else

#   define n_ POTHOS_PP_FRAME_ITERATION(1)

#if !defined(POTHOS_NO_TEMPLATE_PARTIAL_SPECIALIZATION)

template<
      typename First
    , typename Last
    , typename State
    , typename ForwardOp
    >
struct AUX778076_FOLD_IMPL_NAME<n_,First,Last,State,ForwardOp>
{
    typedef First iter0;
    typedef State state0;

    POTHOS_MPL_PP_REPEAT(n_, AUX778076_ITER_FOLD_STEP, unused)

    typedef POTHOS_PP_CAT(state,n_) state;
    typedef POTHOS_PP_CAT(iter,n_) iterator;
};

#else

template<> struct AUX778076_FOLD_CHUNK_NAME<n_>
{
    template<
          typename First
        , typename Last
        , typename State
        , typename ForwardOp
        >
    struct result_
    {
        typedef First iter0;
        typedef State state0;

        POTHOS_MPL_PP_REPEAT(n_, AUX778076_ITER_FOLD_STEP, unused)

        typedef POTHOS_PP_CAT(state,n_) state;
        typedef POTHOS_PP_CAT(iter,n_) iterator;
    };

#if defined(POTHOS_MPL_CFG_MSVC_60_ETI_BUG)
    /// ETI workaround
    template<> struct result_<int,int,int,int>
    {
        typedef int state;
        typedef int iterator;
    };
#endif
};

#endif // BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION

#   undef n_

#endif // BOOST_PP_IS_ITERATING
