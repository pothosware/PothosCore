// Boost.Range library
//
//  Copyright Thorsten Ottosen 2003-2004. Use, modification and
//  distribution is subject to the Boost Software License, Version
//  1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// For more information, see http://www.boost.org/libs/range/
//

#ifndef POTHOS_RANGE_BEGIN_HPP
#define POTHOS_RANGE_BEGIN_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif

#include <Pothos/serialization/impl/range/config.hpp>

#ifdef POTHOS_NO_FUNCTION_TEMPLATE_ORDERING
#include <Pothos/serialization/impl/range/detail/begin.hpp>
#else

#include <Pothos/serialization/impl/range/iterator.hpp>

namespace Pothos
{

#if !POTHOS_WORKAROUND(__BORLANDC__, POTHOS_TESTED_AT(0x564)) && \
    !POTHOS_WORKAROUND(__GNUC__, < 3) \
    /**/
namespace range_detail
{
#endif

    //////////////////////////////////////////////////////////////////////
    // primary template
    //////////////////////////////////////////////////////////////////////

    template< typename C >
    inline POTHOS_DEDUCED_TYPENAME range_iterator<C>::type
    range_begin( C& c )
    {
        //
        // If you get a compile-error here, it is most likely because
        // you have not implemented range_begin() properly in
        // the namespace of C
        //
        return c.begin();
    }

    //////////////////////////////////////////////////////////////////////
    // pair
    //////////////////////////////////////////////////////////////////////

    template< typename Iterator >
    inline Iterator range_begin( const std::pair<Iterator,Iterator>& p )
    {
        return p.first;
    }

    template< typename Iterator >
    inline Iterator range_begin( std::pair<Iterator,Iterator>& p )
    {
        return p.first;
    }

    //////////////////////////////////////////////////////////////////////
    // array
    //////////////////////////////////////////////////////////////////////

    //
    // May this be discarded? Or is it needed for bad compilers?
    //
    template< typename T, std::size_t sz >
    inline const T* range_begin( const T (&a)[sz] )
    {
        return a;
    }

    template< typename T, std::size_t sz >
    inline T* range_begin( T (&a)[sz] )
    {
        return a;
    }


#if !POTHOS_WORKAROUND(__BORLANDC__, POTHOS_TESTED_AT(0x564)) && \
    !POTHOS_WORKAROUND(__GNUC__, < 3) \
    /**/
} // namespace 'range_detail'
#endif

// Use a ADL namespace barrier to avoid ambiguity with other unqualified
// calls. This is particularly important with C++0x encouraging
// unqualified calls to begin/end.
namespace range_adl_barrier
{

template< class T >
inline POTHOS_DEDUCED_TYPENAME range_iterator<T>::type begin( T& r )
{
#if !POTHOS_WORKAROUND(__BORLANDC__, POTHOS_TESTED_AT(0x564)) && \
    !POTHOS_WORKAROUND(__GNUC__, < 3) \
    /**/
    using namespace range_detail;
#endif
    return range_begin( r );
}

template< class T >
inline POTHOS_DEDUCED_TYPENAME range_iterator<const T>::type begin( const T& r )
{
#if !POTHOS_WORKAROUND(__BORLANDC__, POTHOS_TESTED_AT(0x564)) && \
    !POTHOS_WORKAROUND(__GNUC__, < 3) \
    /**/
    using namespace range_detail;
#endif
    return range_begin( r );
}

    } // namespace range_adl_barrier
} // namespace boost

#endif // BOOST_NO_FUNCTION_TEMPLATE_ORDERING

namespace Pothos
{
    namespace range_adl_barrier
    {
        template< class T >
        inline POTHOS_DEDUCED_TYPENAME range_iterator<const T>::type
        const_begin( const T& r )
        {
            return Pothos::range_adl_barrier::begin( r );
        }
    } // namespace range_adl_barrier

    using namespace range_adl_barrier;
} // namespace boost

#endif

