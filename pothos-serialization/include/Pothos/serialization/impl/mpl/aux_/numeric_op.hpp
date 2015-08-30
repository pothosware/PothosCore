
#if !defined(POTHOS_PP_IS_ITERATING)

///// header body

// NO INCLUDE GUARDS, THE HEADER IS INTENDED FOR MULTIPLE INCLUSION!

// Copyright Aleksey Gurtovoy 2000-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: numeric_op.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-10 23:19:02 -0700 (Fri, 10 Oct 2008) $
// $Revision: 49267 $

#if !defined(POTHOS_MPL_PREPROCESSING_MODE)
#   include <Pothos/serialization/impl/mpl/numeric_cast.hpp>
#   include <Pothos/serialization/impl/mpl/apply_wrap.hpp>
#   include <Pothos/serialization/impl/mpl/if.hpp>
#   include <Pothos/serialization/impl/mpl/tag.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/numeric_cast_utils.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/na.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/na_spec.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/lambda_support.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/msvc_eti_base.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/value_wknd.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/config/eti.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/nttp_decl.hpp>
#endif

#include <Pothos/serialization/impl/mpl/aux_/config/static_constant.hpp>

#if defined(POTHOS_MPL_CFG_NO_PREPROCESSED_HEADERS) \
    || defined(POTHOS_MPL_PREPROCESSING_MODE)

#   include <Pothos/serialization/impl/mpl/limits/arity.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/preprocessor/partial_spec_params.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/preprocessor/def_params_tail.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/preprocessor/repeat.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/preprocessor/ext_params.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/preprocessor/params.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/preprocessor/enum.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/preprocessor/add.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/preprocessor/sub.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/config/ctps.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/config/eti.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/config/msvc.hpp>
#   include <Pothos/serialization/impl/mpl/aux_/config/workaround.hpp>

#   include <Pothos/serialization/impl/preprocessor/dec.hpp>
#   include <Pothos/serialization/impl/preprocessor/inc.hpp>
#   include <Pothos/serialization/impl/preprocessor/iterate.hpp>
#   include <Pothos/serialization/impl/preprocessor/cat.hpp>


#if !defined(AUX778076_OP_ARITY)
#   define AUX778076_OP_ARITY POTHOS_MPL_LIMIT_METAFUNCTION_ARITY
#endif

#if !defined(AUX778076_OP_IMPL_NAME)
#   define AUX778076_OP_IMPL_NAME POTHOS_PP_CAT(AUX778076_OP_PREFIX,_impl)
#endif

#if !defined(AUX778076_OP_TAG_NAME)
#   define AUX778076_OP_TAG_NAME POTHOS_PP_CAT(AUX778076_OP_PREFIX,_tag)
#endif

namespace Pothos { namespace mpl {

template< 
      typename Tag1
    , typename Tag2
#if POTHOS_WORKAROUND(POTHOS_MSVC, <= 1300)
    , POTHOS_MPL_AUX_NTTP_DECL(int, tag1_) = POTHOS_MPL_AUX_MSVC_VALUE_WKND(Tag1)::value 
    , POTHOS_MPL_AUX_NTTP_DECL(int, tag2_) = POTHOS_MPL_AUX_MSVC_VALUE_WKND(Tag2)::value 
    >
struct AUX778076_OP_IMPL_NAME
    : if_c<
          ( tag1_ > tag2_ )
#else
    >
struct AUX778076_OP_IMPL_NAME
    : if_c<
          ( POTHOS_MPL_AUX_NESTED_VALUE_WKND(int, Tag1)
              > POTHOS_MPL_AUX_NESTED_VALUE_WKND(int, Tag2)
            )
#endif
        , aux::cast2nd_impl< AUX778076_OP_IMPL_NAME<Tag1,Tag1>,Tag1,Tag2 >
        , aux::cast1st_impl< AUX778076_OP_IMPL_NAME<Tag2,Tag2>,Tag1,Tag2 >
        >::type
{
};

/// for Digital Mars C++/compilers with no CTPS/TTP support
template<> struct AUX778076_OP_IMPL_NAME<na,na>
{
    template< typename U1, typename U2 > struct apply 
    {
        typedef apply type;
        POTHOS_STATIC_CONSTANT(int, value = 0);
    };
};

#if !defined(POTHOS_NO_TEMPLATE_PARTIAL_SPECIALIZATION)
template< typename Tag > struct AUX778076_OP_IMPL_NAME<na,Tag>
{
    template< typename U1, typename U2 > struct apply 
    {
        typedef apply type;
        POTHOS_STATIC_CONSTANT(int, value = 0);
    };
};

template< typename Tag > struct AUX778076_OP_IMPL_NAME<Tag,na>
{
    template< typename U1, typename U2 > struct apply 
    {
        typedef apply type;
        POTHOS_STATIC_CONSTANT(int, value = 0);
    };
};
#else
template<> struct AUX778076_OP_IMPL_NAME<na,integral_c_tag>
{
    template< typename U1, typename U2 > struct apply 
    {
        typedef apply type;
        POTHOS_STATIC_CONSTANT(int, value = 0);
    };
};

template<> struct AUX778076_OP_IMPL_NAME<integral_c_tag,na>
{
    template< typename U1, typename U2 > struct apply 
    {
        typedef apply type;
        POTHOS_STATIC_CONSTANT(int, value = 0);
    };
};
#endif


#if defined(POTHOS_NO_TEMPLATE_PARTIAL_SPECIALIZATION) \
    && POTHOS_WORKAROUND(POTHOS_MSVC, >= 1300)
template< typename T > struct AUX778076_OP_TAG_NAME
    : tag<T,na>
{
};
#else
template< typename T > struct AUX778076_OP_TAG_NAME
{
    typedef typename T::tag type;
};
#endif


#if AUX778076_OP_ARITY != 2

#   if !defined(POTHOS_NO_TEMPLATE_PARTIAL_SPECIALIZATION)

#   define AUX778076_OP_RIGHT_OPERAND(unused, i, N) , POTHOS_PP_CAT(N, POTHOS_MPL_PP_ADD(i, 2))>
#   define AUX778076_OP_N_CALLS(i, N) \
    POTHOS_MPL_PP_REPEAT( POTHOS_PP_DEC(i), POTHOS_MPL_PP_REPEAT_IDENTITY_FUNC, AUX778076_OP_NAME< ) \
    N1 POTHOS_MPL_PP_REPEAT( POTHOS_MPL_PP_SUB(i, 1), AUX778076_OP_RIGHT_OPERAND, N ) \
/**/

template<
      typename POTHOS_MPL_AUX_NA_PARAM(N1)
    , typename POTHOS_MPL_AUX_NA_PARAM(N2)
    POTHOS_MPL_PP_DEF_PARAMS_TAIL(2, typename N, na)
    >
struct AUX778076_OP_NAME
    : AUX778076_OP_N_CALLS(AUX778076_OP_ARITY, N)
{
    POTHOS_MPL_AUX_LAMBDA_SUPPORT(
          AUX778076_OP_ARITY
        , AUX778076_OP_NAME
        , ( POTHOS_MPL_PP_PARAMS(AUX778076_OP_ARITY, N) )
        )
};

#define POTHOS_PP_ITERATION_PARAMS_1 \
    (3,( POTHOS_PP_DEC(AUX778076_OP_ARITY), 2, <boost/mpl/aux_/numeric_op.hpp> ))
#include POTHOS_PP_ITERATE()

#   undef AUX778076_OP_N_CALLS
#   undef AUX778076_OP_RIGHT_OPERAND

#   else // BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION

/// forward declaration
template< 
      typename POTHOS_MPL_AUX_NA_PARAM(N1)
    , typename POTHOS_MPL_AUX_NA_PARAM(N2)
    >
struct POTHOS_PP_CAT(AUX778076_OP_NAME,2);

template<
      typename POTHOS_MPL_AUX_NA_PARAM(N1)
    , typename POTHOS_MPL_AUX_NA_PARAM(N2)
    POTHOS_MPL_PP_DEF_PARAMS_TAIL(2, typename N, na)
    >
struct AUX778076_OP_NAME
#if POTHOS_WORKAROUND(POTHOS_MSVC, == 1300)
    : aux::msvc_eti_base< typename if_<
#else
    : if_<
#endif
          is_na<N3>
        , POTHOS_PP_CAT(AUX778076_OP_NAME,2)<N1,N2>
        , AUX778076_OP_NAME<
              POTHOS_PP_CAT(AUX778076_OP_NAME,2)<N1,N2>
            , POTHOS_MPL_PP_EXT_PARAMS(3, POTHOS_PP_INC(AUX778076_OP_ARITY), N)
            >
        >::type
#if POTHOS_WORKAROUND(POTHOS_MSVC, == 1300)
    >
#endif
{
    POTHOS_MPL_AUX_LAMBDA_SUPPORT(
          AUX778076_OP_ARITY
        , AUX778076_OP_NAME
        , ( POTHOS_MPL_PP_PARAMS(AUX778076_OP_ARITY, N) )
        )
};

template< 
      typename N1
    , typename N2
    >
struct POTHOS_PP_CAT(AUX778076_OP_NAME,2)

#endif

#else // AUX778076_OP_ARITY == 2

template< 
      typename POTHOS_MPL_AUX_NA_PARAM(N1)
    , typename POTHOS_MPL_AUX_NA_PARAM(N2)
    >
struct AUX778076_OP_NAME

#endif

#if !defined(POTHOS_MPL_CFG_MSVC_ETI_BUG)
    : AUX778076_OP_IMPL_NAME<
          typename AUX778076_OP_TAG_NAME<N1>::type
        , typename AUX778076_OP_TAG_NAME<N2>::type
        >::template apply<N1,N2>::type
#else
    : aux::msvc_eti_base< typename apply_wrap2<
          AUX778076_OP_IMPL_NAME<
              typename AUX778076_OP_TAG_NAME<N1>::type
            , typename AUX778076_OP_TAG_NAME<N2>::type
            >
        , N1
        , N2
        >::type >::type
#endif
{
#if AUX778076_OP_ARITY != 2

#   if !defined(POTHOS_NO_TEMPLATE_PARTIAL_SPECIALIZATION)
    POTHOS_MPL_AUX_LAMBDA_SUPPORT_SPEC(
          AUX778076_OP_ARITY
        , AUX778076_OP_NAME
        , ( POTHOS_MPL_PP_PARTIAL_SPEC_PARAMS(2, N, na) )
        )
#   else
    POTHOS_MPL_AUX_LAMBDA_SUPPORT(2, POTHOS_PP_CAT(AUX778076_OP_NAME,2), (N1, N2))
#   endif

#else
    POTHOS_MPL_AUX_LAMBDA_SUPPORT(2, AUX778076_OP_NAME, (N1, N2))
#endif
};

POTHOS_MPL_AUX_NA_SPEC2(2, AUX778076_OP_ARITY, AUX778076_OP_NAME)

}}

#endif // BOOST_MPL_CFG_NO_PREPROCESSED_HEADERS

///// iteration, depth == 1

// For gcc 4.4 compatability, we must include the
// BOOST_PP_ITERATION_DEPTH test inside an #else clause.
#else // BOOST_PP_IS_ITERATING
#if POTHOS_PP_ITERATION_DEPTH() == 1

#   define i_ POTHOS_PP_FRAME_ITERATION(1)

template<
      POTHOS_MPL_PP_PARAMS(i_, typename N)
    >
struct AUX778076_OP_NAME<POTHOS_MPL_PP_PARTIAL_SPEC_PARAMS(i_, N, na)>
#if i_ != 2
    : AUX778076_OP_N_CALLS(i_, N)
{
    POTHOS_MPL_AUX_LAMBDA_SUPPORT_SPEC(
          AUX778076_OP_ARITY
        , AUX778076_OP_NAME
        , ( POTHOS_MPL_PP_PARTIAL_SPEC_PARAMS(i_, N, na) )
        )
};
#endif

#   undef i_

#endif // BOOST_PP_ITERATION_DEPTH()
#endif // BOOST_PP_IS_ITERATING
