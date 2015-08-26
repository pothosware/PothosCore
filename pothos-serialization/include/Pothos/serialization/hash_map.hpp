#ifndef  POTHOS_SERIALIZATION_HASH_MAP_HPP
#define POTHOS_SERIALIZATION_HASH_MAP_HPP

// MS compatible compilers support #pragma once
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// serialization/hash_map.hpp:
// serialization for stl hash_map templates

// (C) Copyright 2002 Robert Ramey - http://www.rrsd.com . 
// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for updates, documentation, and revision history.

#include <Pothos/serialization/impl/config.hpp>
#ifdef POTHOS_HAS_HASH
#include POTHOS_HASH_MAP_HEADER

#include <Pothos/serialization/utility.hpp>
#include <Pothos/serialization/hash_collections_save_imp.hpp>
#include <Pothos/serialization/hash_collections_load_imp.hpp>
#include <Pothos/serialization/split_free.hpp>

namespace Pothos { 
namespace serialization {

namespace stl {

// map input
template<class Archive, class Container>
struct archive_input_hash_map
{
    inline void operator()(
        Archive &ar, 
        Container &s, 
        const unsigned int v
    ){
        typedef POTHOS_DEDUCED_TYPENAME Container::value_type type;
        detail::stack_construct<Archive, type> t(ar, v);
        // borland fails silently w/o full namespace
        ar >> Pothos::serialization::make_nvp("item", t.reference());
        std::pair<POTHOS_DEDUCED_TYPENAME Container::const_iterator, bool> result = 
            s.insert(t.reference());
        // note: the following presumes that the map::value_type was NOT tracked
        // in the archive.  This is the usual case, but here there is no way
        // to determine that.  
        if(result.second){
            ar.reset_object_address(
                & (result.first->second),
                & t.reference().second
            );
        }
    }
};

// multimap input
template<class Archive, class Container>
struct archive_input_hash_multimap
{
    inline void operator()(
        Archive &ar, 
        Container &s, 
        const unsigned int v
    ){
        typedef POTHOS_DEDUCED_TYPENAME Container::value_type type;
        detail::stack_construct<Archive, type> t(ar, v);
        // borland fails silently w/o full namespace
        ar >> Pothos::serialization::make_nvp("item", t.reference());
        POTHOS_DEDUCED_TYPENAME Container::const_iterator result 
            = s.insert(t.reference());
        // note: the following presumes that the map::value_type was NOT tracked
        // in the archive.  This is the usual case, but here there is no way
        // to determine that.  
        ar.reset_object_address(
            & result->second,
            & t.reference()
        );
    }
};

} // stl

template<
    class Archive, 
    class Key, 
    class HashFcn, 
    class EqualKey,
    class Allocator
>
inline void save(
    Archive & ar,
    const POTHOS_STD_EXTENSION_NAMESPACE::hash_map<
        Key, HashFcn, EqualKey, Allocator
    > &t,
    const unsigned int file_version
){
    Pothos::serialization::stl::save_hash_collection<
        Archive, 
        POTHOS_STD_EXTENSION_NAMESPACE::hash_map<
            Key, HashFcn, EqualKey, Allocator
        >
    >(ar, t);
}

template<
    class Archive, 
    class Key, 
    class HashFcn, 
    class EqualKey,
    class Allocator
>
inline void load(
    Archive & ar,
    POTHOS_STD_EXTENSION_NAMESPACE::hash_map<
        Key, HashFcn, EqualKey, Allocator
    > &t,
    const unsigned int file_version
){
    Pothos::serialization::stl::load_hash_collection<
        Archive,
        POTHOS_STD_EXTENSION_NAMESPACE::hash_map<
            Key, HashFcn, EqualKey, Allocator
        >,
        Pothos::serialization::stl::archive_input_hash_map<
            Archive, 
            POTHOS_STD_EXTENSION_NAMESPACE::hash_map<
                Key, HashFcn, EqualKey, Allocator
            >
        >
    >(ar, t);
}

// split non-intrusive serialization function member into separate
// non intrusive save/load member functions
template<
    class Archive, 
    class Key, 
    class HashFcn, 
    class EqualKey,
    class Allocator
>
inline void serialize(
    Archive & ar,
    POTHOS_STD_EXTENSION_NAMESPACE::hash_map<
        Key, HashFcn, EqualKey, Allocator
    > &t,
    const unsigned int file_version
){
    Pothos::serialization::split_free(ar, t, file_version);
}

// hash_multimap
template<
    class Archive, 
    class Key, 
    class HashFcn, 
    class EqualKey,
    class Allocator
>
inline void save(
    Archive & ar,
    const POTHOS_STD_EXTENSION_NAMESPACE::hash_multimap<
        Key, HashFcn, EqualKey, Allocator
    > &t,
    const unsigned int file_version
){
    Pothos::serialization::stl::save_hash_collection<
        Archive, 
        POTHOS_STD_EXTENSION_NAMESPACE::hash_multimap<
            Key, HashFcn, EqualKey, Allocator
        >
    >(ar, t);
}

template<
    class Archive, 
    class Key, 
    class HashFcn, 
    class EqualKey,
    class Allocator
>
inline void load(
    Archive & ar,
    POTHOS_STD_EXTENSION_NAMESPACE::hash_multimap<
        Key, HashFcn, EqualKey, Allocator
    > &t,
    const unsigned int file_version
){
    Pothos::serialization::stl::load_hash_collection<
        Archive,
        POTHOS_STD_EXTENSION_NAMESPACE::hash_multimap<
            Key, HashFcn, EqualKey, Allocator
        >,
        Pothos::serialization::stl::archive_input_hash_multimap<
            Archive, 
            POTHOS_STD_EXTENSION_NAMESPACE::hash_multimap<
                Key, HashFcn, EqualKey, Allocator
            >
        >
    >(ar, t);
}

// split non-intrusive serialization function member into separate
// non intrusive save/load member functions
template<
    class Archive, 
    class Key, 
    class HashFcn, 
    class EqualKey,
    class Allocator
>
inline void serialize(
    Archive & ar,
    POTHOS_STD_EXTENSION_NAMESPACE::hash_multimap<
        Key, HashFcn, EqualKey, Allocator
    > &t,
    const unsigned int file_version
){
    Pothos::serialization::split_free(ar, t, file_version);
}

} // namespace serialization
} // namespace boost

#endif // BOOST_HAS_HASH
#endif // BOOST_SERIALIZATION_HASH_MAP_HPP
