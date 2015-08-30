// Boost.Range library
//
//  Copyright Thorsten Ottosen 2003-2004. Use, modification and
//  distribution is subject to the Boost Software License, Version
//  1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// For more information, see http://www.boost.org/libs/range/
//

#ifndef POTHOS_RANGE_DETAIL_END_HPP
#define POTHOS_RANGE_DETAIL_END_HPP

#include <Pothos/serialization/impl/config.hpp> // BOOST_MSVC
#include <Pothos/serialization/impl/detail/workaround.hpp>

#if POTHOS_WORKAROUND(POTHOS_MSVC, < 1300)
# include <Pothos/serialization/impl/range/detail/vc6/end.hpp>
#else
# include <Pothos/serialization/impl/range/detail/implementation_help.hpp>
# include <Pothos/serialization/impl/range/iterator.hpp>
# include <Pothos/serialization/impl/range/detail/common.hpp>
# if POTHOS_WORKAROUND(POTHOS_MSVC, < 1310)
#  include <Pothos/serialization/impl/range/detail/remove_extent.hpp>
# endif

namespace Pothos
{
    namespace range_detail
    {
        template< typename T >
        struct range_end;

        //////////////////////////////////////////////////////////////////////
        // default
        //////////////////////////////////////////////////////////////////////

        template<>
        struct range_end<std_container_>
        {
            template< typename C >
            static POTHOS_RANGE_DEDUCED_TYPENAME range_iterator<C>::type
            fun( C& c )
            {
                return c.end();
            };
        };

        //////////////////////////////////////////////////////////////////////
        // pair
        //////////////////////////////////////////////////////////////////////

        template<>
        struct range_end<std_pair_>
        {
            template< typename P >
            static POTHOS_RANGE_DEDUCED_TYPENAME range_iterator<P>::type
            fun( const P& p )
            {
                return p.second;
            }
        };

        //////////////////////////////////////////////////////////////////////
        // array
        //////////////////////////////////////////////////////////////////////

        template<>
        struct range_end<array_>
        {
        #if !POTHOS_WORKAROUND(POTHOS_MSVC, < 1310)
            template< typename T, std::size_t sz >
            static T* fun( T POTHOS_RANGE_ARRAY_REF()[sz] )
            {
                return Pothos::range_detail::array_end( Pothos_range_array );
            }
        #else
            template<typename T>
            static POTHOS_RANGE_DEDUCED_TYPENAME remove_extent<T>::type* fun(T& t)
            {
                return t + remove_extent<T>::size;
            }
        #endif
        };

    } // namespace 'range_detail'

    namespace range_adl_barrier
    {
        template< typename C >
        inline POTHOS_RANGE_DEDUCED_TYPENAME range_iterator<C>::type
        end( C& c )
        {
            return range_detail::range_end< POTHOS_RANGE_DEDUCED_TYPENAME range_detail::range<C>::type >::fun( c );
        }
    } // namespace range_adl_barrier

} // namespace 'boost'

# endif // VC6
#endif
