// Boost.Range library
//
//  Copyright Thorsten Ottosen 2003-2004. Use, modification and
//  distribution is subject to the Boost Software License, Version
//  1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// For more information, see http://www.boost.org/libs/range/
//

#ifndef POTHOS_RANGE_ITERATOR_HPP
#define POTHOS_RANGE_ITERATOR_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#include <Pothos/serialization/impl/range/config.hpp>
#include <Pothos/serialization/impl/range/mutable_iterator.hpp>
#include <Pothos/serialization/impl/range/const_iterator.hpp>
#include <Pothos/serialization/impl/type_traits/is_const.hpp>
#include <Pothos/serialization/impl/type_traits/remove_const.hpp>
#include <Pothos/serialization/impl/mpl/eval_if.hpp>

namespace Pothos
{

#if POTHOS_WORKAROUND(POTHOS_MSVC, == 1310)

    namespace range_detail_vc7_1
    {
       template< typename C, typename Sig = void(C) >
       struct range_iterator
       {
           typedef POTHOS_RANGE_DEDUCED_TYPENAME
               mpl::eval_if_c< is_const<C>::value,
                               range_const_iterator< typename remove_const<C>::type >,
                               range_mutable_iterator<C> >::type type;
       };

       template< typename C, typename T >
       struct range_iterator< C, void(T[]) >
       {
           typedef T* type;
       };
    }

#endif

    template< typename C >
    struct range_iterator
    {
#if POTHOS_WORKAROUND(POTHOS_MSVC, == 1310)

        typedef POTHOS_RANGE_DEDUCED_TYPENAME
               range_detail_vc7_1::range_iterator<C>::type type;

#else

        typedef POTHOS_RANGE_DEDUCED_TYPENAME
            mpl::eval_if_c< is_const<C>::value,
                            range_const_iterator< typename remove_const<C>::type >,
                            range_mutable_iterator<C> >::type type;

#endif
    };

} // namespace boost

//#endif // BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION

#endif
