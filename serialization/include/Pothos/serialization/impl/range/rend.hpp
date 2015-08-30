// Boost.Range library
//
//  Copyright Thorsten Ottosen 2003-2004. Use, modification and
//  distribution is subject to the Boost Software License, Version
//  1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// For more information, see http://www.boost.org/libs/range/
//

#ifndef POTHOS_RANGE_REND_HPP
#define POTHOS_RANGE_REND_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif

#include <Pothos/serialization/impl/range/begin.hpp>
#include <Pothos/serialization/impl/range/reverse_iterator.hpp>

namespace Pothos
{

#ifdef POTHOS_NO_FUNCTION_TEMPLATE_ORDERING

template< class C >
inline POTHOS_DEDUCED_TYPENAME range_reverse_iterator<C>::type
rend( C& c )
{
    return POTHOS_DEDUCED_TYPENAME range_reverse_iterator<C>::type( Pothos::begin( c ) );
}

#else

template< class C >
inline POTHOS_DEDUCED_TYPENAME range_reverse_iterator<C>::type
rend( C& c )
{
    typedef POTHOS_DEDUCED_TYPENAME range_reverse_iterator<C>::type
               iter_type;
    return iter_type( Pothos::begin( c ) );
}

template< class C >
inline POTHOS_DEDUCED_TYPENAME range_reverse_iterator<const C>::type
rend( const C& c )
{
    typedef POTHOS_DEDUCED_TYPENAME range_reverse_iterator<const C>::type
        iter_type;
    return iter_type( Pothos::begin( c ) );
}

#endif

template< class T >
inline POTHOS_DEDUCED_TYPENAME range_reverse_iterator<const T>::type
const_rend( const T& r )
{
    return Pothos::rend( r );
}

} // namespace 'boost'

#endif

