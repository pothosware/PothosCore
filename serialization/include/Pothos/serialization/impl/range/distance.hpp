// Boost.Range library
//
//  Copyright Thorsten Ottosen 2003-2006. Use, modification and
//  distribution is subject to the Boost Software License, Version
//  1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// For more information, see http://www.boost.org/libs/range/
//

#ifndef POTHOS_RANGE_DISTANCE_HPP
#define POTHOS_RANGE_DISTANCE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif

#include <Pothos/serialization/impl/range/begin.hpp>
#include <Pothos/serialization/impl/range/end.hpp>
#include <Pothos/serialization/impl/range/difference_type.hpp>

namespace Pothos 
{

    template< class T >
    inline POTHOS_DEDUCED_TYPENAME range_difference<T>::type 
    distance( const T& r )
    {
        return std::distance( Pothos::begin( r ), Pothos::end( r ) );
    }

} // namespace 'boost'

#endif
