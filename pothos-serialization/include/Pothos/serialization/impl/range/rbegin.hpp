// Boost.Range library
//
//  Copyright Thorsten Ottosen 2003-2004. Use, modification and
//  distribution is subject to the Boost Software License, Version
//  1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// For more information, see http://www.boost.org/libs/range/
//

#ifndef POTHOS_RANGE_RBEGIN_HPP
#define POTHOS_RANGE_RBEGIN_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif

#include <Pothos/serialization/impl/range/end.hpp>
#include <Pothos/serialization/impl/range/reverse_iterator.hpp>

namespace Pothos
{

#ifdef POTHOS_NO_FUNCTION_TEMPLATE_ORDERING

template< class C >
inline POTHOS_DEDUCED_TYPENAME range_reverse_iterator<C>::type
rbegin( C& c )
{
    return POTHOS_DEDUCED_TYPENAME range_reverse_iterator<C>::type( Pothos::end( c ) );
}

#else

template< class C >
inline POTHOS_DEDUCED_TYPENAME range_reverse_iterator<C>::type
rbegin( C& c )
{
    typedef POTHOS_DEDUCED_TYPENAME range_reverse_iterator<C>::type
        iter_type;
    return iter_type( Pothos::end( c ) );
}

template< class C >
inline POTHOS_DEDUCED_TYPENAME range_reverse_iterator<const C>::type
rbegin( const C& c )
{
    typedef POTHOS_DEDUCED_TYPENAME range_reverse_iterator<const C>::type
        iter_type;
    return iter_type( Pothos::end( c ) );
}

#endif // BOOST_NO_FUNCTION_TEMPLATE_ORDERING

template< class T >
inline POTHOS_DEDUCED_TYPENAME range_reverse_iterator<const T>::type
const_rbegin( const T& r )
{
    return Pothos::rbegin( r );
}

} // namespace 'boost'

#endif

