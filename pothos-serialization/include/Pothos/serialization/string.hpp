#ifndef  POTHOS_SERIALIZATION_STRING_HPP
#define POTHOS_SERIALIZATION_STRING_HPP

// MS compatible compilers support #pragma once
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// serialization/string.hpp:
// serialization for stl string templates

// (C) Copyright 2002 Robert Ramey - http://www.rrsd.com . 
// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for updates, documentation, and revision history.

#include <string>

#include <Pothos/serialization/impl/config.hpp>
#include <Pothos/serialization/level.hpp>

POTHOS_CLASS_IMPLEMENTATION(std::string, Pothos::serialization::primitive_type)
#ifndef POTHOS_NO_STD_WSTRING
POTHOS_CLASS_IMPLEMENTATION(std::wstring, Pothos::serialization::primitive_type)
#endif

// left over from a previous incarnation - strings are now always primitive types
#if 0 
#include <string>
#include <Pothos/serialization/collections_save_imp.hpp>
#include <Pothos/serialization/collections_load_imp.hpp>
#include <Pothos/serialization/split_free.hpp>

namespace Pothos { 
namespace serialization {

// basic_string - general case
template<class Archive, class U, class Allocator>
inline void save(
    Archive & ar,
    const std::basic_string<U, Allocator> &t,
    const unsigned int file_version
){
    Pothos::serialization::stl::save_collection<
        Archive, std::basic_string<U, Allocator> 
    >(ar, t);
}

template<class Archive, class U, class Allocator>
inline void load(
    Archive & ar,
    std::basic_string<U, Allocator> &t,
    const unsigned int file_version
){
    Pothos::serialization::stl::load_collection<
        Archive,
        std::basic_string<U, Allocator>,
        Pothos::serialization::stl::archive_input_seq<
            Archive, 
            std::basic_string<U, Allocator> 
        >,
        Pothos::serialization::stl::reserve_imp<
            std::basic_string<U, Allocator> 
        >
    >(ar, t);
}

// split non-intrusive serialization function member into separate
// non intrusive save/load member functions
template<class Archive, class U, class Allocator>
inline void serialize(
    Archive & ar,
    std::basic_string<U, Allocator> & t,
    const unsigned int file_version
){
    Pothos::serialization::split_free(ar, t, file_version);
}

} // serialization
} // namespace boost

#include <Pothos/serialization/collection_traits.hpp>

POTHOS_SERIALIZATION_COLLECTION_TRAITS(std::vector)

#endif

#endif // BOOST_SERIALIZATION_STRING_HPP
