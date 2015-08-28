#ifndef  POTHOS_SERIALIZATION_SET_HPP
#define POTHOS_SERIALIZATION_SET_HPP

// MS compatible compilers support #pragma once
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// set.hpp: serialization for stl set templates

// (C) Copyright 2002 Robert Ramey - http://www.rrsd.com . 
// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for updates, documentation, and revision history.

#include <set>

#include <Pothos/serialization/impl/config.hpp>

#include <Pothos/serialization/collections_save_imp.hpp>
#include <Pothos/serialization/collections_load_imp.hpp>
#include <Pothos/serialization/split_free.hpp>

namespace Pothos { 
namespace serialization {

template<class Archive, class Key, class Compare, class Allocator >
inline void save(
    Archive & ar,
    const std::set<Key, Compare, Allocator> &t,
    const unsigned int /* file_version */
){
    Pothos::serialization::stl::save_collection<
        Archive, std::set<Key, Compare, Allocator> 
    >(ar, t);
}

template<class Archive, class Key, class Compare, class Allocator >
inline void load(
    Archive & ar,
    std::set<Key, Compare, Allocator> &t,
    const unsigned int /* file_version */
){
    Pothos::serialization::stl::load_collection<
        Archive,
        std::set<Key, Compare, Allocator>,
        Pothos::serialization::stl::archive_input_set<
            Archive, std::set<Key, Compare, Allocator> 
        >,
        Pothos::serialization::stl::no_reserve_imp<std::set<
            Key, Compare, Allocator> 
        >
    >(ar, t);
}

// split non-intrusive serialization function member into separate
// non intrusive save/load member functions
template<class Archive, class Key, class Compare, class Allocator >
inline void serialize(
    Archive & ar,
    std::set<Key, Compare, Allocator> & t,
    const unsigned int file_version
){
    Pothos::serialization::split_free(ar, t, file_version);
}

// multiset
template<class Archive, class Key, class Compare, class Allocator >
inline void save(
    Archive & ar,
    const std::multiset<Key, Compare, Allocator> &t,
    const unsigned int /* file_version */
){
    Pothos::serialization::stl::save_collection<
        Archive, 
        std::multiset<Key, Compare, Allocator> 
    >(ar, t);
}

template<class Archive, class Key, class Compare, class Allocator >
inline void load(
    Archive & ar,
    std::multiset<Key, Compare, Allocator> &t,
    const unsigned int /* file_version */
){
    Pothos::serialization::stl::load_collection<
        Archive,
        std::multiset<Key, Compare, Allocator>,
        Pothos::serialization::stl::archive_input_set<
            Archive, std::multiset<Key, Compare, Allocator> 
        >,
        Pothos::serialization::stl::no_reserve_imp<
            std::multiset<Key, Compare, Allocator> 
        >
    >(ar, t);
}

// split non-intrusive serialization function member into separate
// non intrusive save/load member functions
template<class Archive, class Key, class Compare, class Allocator >
inline void serialize(
    Archive & ar,
    std::multiset<Key, Compare, Allocator> & t,
    const unsigned int file_version
){
    Pothos::serialization::split_free(ar, t, file_version);
}

} // namespace serialization
} // namespace boost

#include <Pothos/serialization/collection_traits.hpp>

POTHOS_SERIALIZATION_COLLECTION_TRAITS(std::set)
POTHOS_SERIALIZATION_COLLECTION_TRAITS(std::multiset)

#endif // BOOST_SERIALIZATION_SET_HPP
