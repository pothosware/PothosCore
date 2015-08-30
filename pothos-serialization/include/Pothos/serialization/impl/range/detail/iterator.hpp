// Boost.Range library
//
//  Copyright Thorsten Ottosen 2003-2004. Use, modification and
//  distribution is subject to the Boost Software License, Version
//  1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// For more information, see http://www.boost.org/libs/range/
//

#ifndef POTHOS_RANGE_DETAIL_ITERATOR_HPP
#define POTHOS_RANGE_DETAIL_ITERATOR_HPP

#include <Pothos/serialization/impl/range/detail/common.hpp>
#include <Pothos/serialization/impl/range/detail/remove_extent.hpp>

#include <Pothos/serialization/impl/static_assert.hpp>

//////////////////////////////////////////////////////////////////////////////
// missing partial specialization  workaround.
//////////////////////////////////////////////////////////////////////////////

namespace Pothos 
{
    namespace range_detail 
    {        
        template< typename T >
        struct range_iterator_ {
            template< typename C >
            struct pts
            {
                typedef int type;
            };
        };

        template<>
        struct range_iterator_<std_container_>
        {
            template< typename C >
            struct pts
            {
                typedef POTHOS_RANGE_DEDUCED_TYPENAME C::iterator type;
            };
        };

        template<>
        struct range_iterator_<std_pair_>
        {
            template< typename P >
            struct pts
            {
                typedef POTHOS_RANGE_DEDUCED_TYPENAME P::first_type type;
            };
        };

        template<>
        struct range_iterator_<array_>
        { 
            template< typename T >
            struct pts
            {
                typedef POTHOS_RANGE_DEDUCED_TYPENAME 
                    remove_extent<T>::type* type;
            };
        };
        
    } 

    template< typename C >
    class range_mutable_iterator
    {
        typedef POTHOS_RANGE_DEDUCED_TYPENAME range_detail::range<C>::type c_type;
    public:
        typedef typename range_detail::range_iterator_<c_type>::POTHOS_NESTED_TEMPLATE pts<C>::type type; 
    };
}

#endif
