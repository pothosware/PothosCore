
// Copyright Aleksey Gurtovoy 2000-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//

// Preprocessed version of "boost/mpl/and.hpp" header
// -- DO NOT modify by hand!

namespace Pothos { namespace mpl {

namespace aux {
template< bool C_ > struct and_impl
{
    template<
          typename T1, typename T2, typename T3, typename T4
        >
    struct result_
        : false_
    {
    };
};

template<> struct and_impl<true>
{
    template<
          typename T1, typename T2, typename T3, typename T4
        >
    struct result_
        : and_impl<
              POTHOS_MPL_AUX_NESTED_TYPE_WKND(T1)::value
            >::template result_< T2,T3,T4,true_ >
    {
    };
};

template<>
struct and_impl<true>
    ::result_< true_,true_,true_,true_ >
        : true_
{
};

} // namespace aux

template<
      typename POTHOS_MPL_AUX_NA_PARAM(T1)
    , typename POTHOS_MPL_AUX_NA_PARAM(T2)
    , typename T3 = true_, typename T4 = true_, typename T5 = true_
    >
struct and_

    : aux::and_impl<
          POTHOS_MPL_AUX_NESTED_TYPE_WKND(T1)::value
        >::template result_< T2,T3,T4,T5 >

{
    POTHOS_MPL_AUX_LAMBDA_SUPPORT(
          5
        , and_
        , ( T1, T2, T3, T4, T5)
        )
};

POTHOS_MPL_AUX_NA_SPEC2(
      2
    , 5
    , and_
    )

}}
