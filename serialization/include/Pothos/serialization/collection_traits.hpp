#ifndef POTHOS_SERIALIZATION_COLLECTION_TRAITS_HPP
#define POTHOS_SERIALIZATION_COLLECTION_TRAITS_HPP

// MS compatible compilers support #pragma once
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// collection_traits.hpp:

// (C) Copyright 2002 Robert Ramey - http://www.rrsd.com . 
// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for updates, documentation, and revision history.

// This header assigns a level implemenation trait to a collection type
// for all primitives.  It is needed so that archives which are meant to be
// portable don't write class information in the archive.  Since, not all
// compiles recognize the same set of primitive types, the possibility
// exists for archives to be non-portable if class information for primitive
// types is included.  This is addressed by the following macros.
#include <Pothos/serialization/impl/config.hpp>
#include <Pothos/serialization/impl/mpl/integral_c.hpp>
#include <Pothos/serialization/impl/mpl/integral_c_tag.hpp>

#include <Pothos/serialization/impl/cstdint.hpp>
#include <Pothos/serialization/impl/integer_traits.hpp>
#include <climits> // ULONG_MAX
#include <Pothos/serialization/level.hpp>

#define POTHOS_SERIALIZATION_COLLECTION_TRAITS_HELPER(T, C)          \
template<>                                                          \
struct implementation_level< C < T > > {                            \
    typedef mpl::integral_c_tag tag;                                \
    typedef mpl::int_<object_serializable> type;                    \
    POTHOS_STATIC_CONSTANT(int, value = object_serializable);        \
};                                                                  \
/**/

#if defined(POTHOS_NO_CWCHAR) || defined(POTHOS_NO_INTRINSIC_WCHAR_T)
    #define POTHOS_SERIALIZATION_COLLECTION_TRAITS_HELPER_WCHAR(C)
#else
    #define POTHOS_SERIALIZATION_COLLECTION_TRAITS_HELPER_WCHAR(C)   \
    POTHOS_SERIALIZATION_COLLECTION_TRAITS_HELPER(wchar_t, C)        \
    /**/
#endif

#if defined(POTHOS_HAS_LONG_LONG)
    #define POTHOS_SERIALIZATION_COLLECTION_TRAITS_HELPER_INT64(C)    \
    POTHOS_SERIALIZATION_COLLECTION_TRAITS_HELPER(Pothos::long_long_type, C)  \
    POTHOS_SERIALIZATION_COLLECTION_TRAITS_HELPER(Pothos::ulong_long_type, C) \
    /**/
#else
    #define POTHOS_SERIALIZATION_COLLECTION_TRAITS_HELPER_INT64(C)
#endif

#define POTHOS_SERIALIZATION_COLLECTION_TRAITS(C)                     \
    namespace Pothos { namespace serialization {                      \
    POTHOS_SERIALIZATION_COLLECTION_TRAITS_HELPER(bool, C)            \
    POTHOS_SERIALIZATION_COLLECTION_TRAITS_HELPER(char, C)            \
    POTHOS_SERIALIZATION_COLLECTION_TRAITS_HELPER(signed char, C)     \
    POTHOS_SERIALIZATION_COLLECTION_TRAITS_HELPER(unsigned char, C)   \
    POTHOS_SERIALIZATION_COLLECTION_TRAITS_HELPER(signed int, C)      \
    POTHOS_SERIALIZATION_COLLECTION_TRAITS_HELPER(unsigned int, C)    \
    POTHOS_SERIALIZATION_COLLECTION_TRAITS_HELPER(signed long, C)     \
    POTHOS_SERIALIZATION_COLLECTION_TRAITS_HELPER(unsigned long, C)   \
    POTHOS_SERIALIZATION_COLLECTION_TRAITS_HELPER(float, C)           \
    POTHOS_SERIALIZATION_COLLECTION_TRAITS_HELPER(double, C)          \
    POTHOS_SERIALIZATION_COLLECTION_TRAITS_HELPER(unsigned short, C)  \
    POTHOS_SERIALIZATION_COLLECTION_TRAITS_HELPER(signed short, C)    \
    POTHOS_SERIALIZATION_COLLECTION_TRAITS_HELPER_INT64(C)            \
    POTHOS_SERIALIZATION_COLLECTION_TRAITS_HELPER_WCHAR(C)            \
    } }                                                              \
    /**/

#endif // BOOST_SERIALIZATION_COLLECTION_TRAITS
