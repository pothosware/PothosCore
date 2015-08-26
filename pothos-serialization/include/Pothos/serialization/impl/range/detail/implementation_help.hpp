// Boost.Range library
//
//  Copyright Thorsten Ottosen 2003-2004. Use, modification and
//  distribution is subject to the Boost Software License, Version
//  1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// For more information, see http://www.boost.org/libs/range/
//

#ifndef POTHOS_RANGE_DETAIL_IMPLEMENTATION_HELP_HPP
#define POTHOS_RANGE_DETAIL_IMPLEMENTATION_HELP_HPP

#include <Pothos/serialization/impl/range/config.hpp>
#include <Pothos/serialization/impl/range/detail/common.hpp>
#include <Pothos/serialization/impl/type_traits/is_same.hpp>
#include <cstddef>
#include <string.h>

#ifndef POTHOS_NO_CWCHAR
#include <wchar.h>
#endif

namespace Pothos
{
    namespace range_detail
    {
        template <typename T>
        inline void Pothos_range_silence_warning( const T& ) { }

        /////////////////////////////////////////////////////////////////////
        // end() help
        /////////////////////////////////////////////////////////////////////

        inline const char* str_end( const char* s, const char* )
        {
            return s + strlen( s );
        }

#ifndef POTHOS_NO_CWCHAR
        inline const wchar_t* str_end( const wchar_t* s, const wchar_t* )
        {
            return s + wcslen( s );
        }
#else
        inline const wchar_t* str_end( const wchar_t* s, const wchar_t* )
        {
            if( s == 0 || s[0] == 0 )
                return s;
            while( *++s != 0 )
                ;
            return s;
        }
#endif

        template< class Char >
        inline Char* str_end( Char* s )
        {
            return const_cast<Char*>( str_end( s, s ) );
        }

        template< class T, std::size_t sz >
        inline T* array_end( T POTHOS_RANGE_ARRAY_REF()[sz] )
        {
            return Pothos_range_array + sz;
        }

        template< class T, std::size_t sz >
        inline const T* array_end( const T POTHOS_RANGE_ARRAY_REF()[sz] )
        {
            return Pothos_range_array + sz;
        }

        /////////////////////////////////////////////////////////////////////
        // size() help
        /////////////////////////////////////////////////////////////////////

        template< class Char >
        inline std::size_t str_size( const Char* const& s )
        {
            return str_end( s ) - s;
        }

        template< class T, std::size_t sz >
        inline std::size_t array_size( T POTHOS_RANGE_ARRAY_REF()[sz] )
        {
            Pothos_range_silence_warning( Pothos_range_array );
            return sz;
        }

        template< class T, std::size_t sz >
        inline std::size_t array_size( const T POTHOS_RANGE_ARRAY_REF()[sz] )
        {
            Pothos_range_silence_warning( Pothos_range_array );
            return sz;
        }

    } // namespace 'range_detail'

} // namespace 'boost'


#endif
