#ifndef POTHOS_SERIALIZATION_SLIST_HPP
#define POTHOS_SERIALIZATION_SLIST_HPP

// MS compatible compilers support #pragma once
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// slist.hpp

// (C) Copyright 2002 Robert Ramey - http://www.rrsd.com . 
// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for updates, documentation, and revision history.

#include <cstddef> // size_t
#include <Pothos/serialization/impl/config.hpp> // msvc 6.0 needs this for warning suppression
#if defined(POTHOS_NO_STDC_NAMESPACE)
namespace std{ 
    using ::size_t; 
} // namespace std
#endif

#ifdef POTHOS_HAS_SLIST
#include POTHOS_SLIST_HEADER

#include <Pothos/serialization/collections_save_imp.hpp>
#include <Pothos/serialization/collections_load_imp.hpp>
#include <Pothos/serialization/split_free.hpp>
#include <Pothos/serialization/nvp.hpp>

namespace Pothos { 
namespace serialization {

template<class Archive, class U, class Allocator>
inline void save(
    Archive & ar,
    const POTHOS_STD_EXTENSION_NAMESPACE::slist<U, Allocator> &t,
    const unsigned int file_version
){
    Pothos::serialization::stl::save_collection<
        Archive,
        POTHOS_STD_EXTENSION_NAMESPACE::slist<U, Allocator> 
    >(ar, t);
}

template<class Archive, class U, class Allocator>
inline void load(
    Archive & ar,
    POTHOS_STD_EXTENSION_NAMESPACE::slist<U, Allocator> &t,
    const unsigned int file_version
){
    // retrieve number of elements
    t.clear();
    // retrieve number of elements
    collection_size_type count;
    ar >> POTHOS_SERIALIZATION_NVP(count);
    if(collection_size_type(0) == count)
        return;
    item_version_type item_version(0);
    const Pothos::archive::library_version_type library_version(
        ar.get_library_version()
    );
    if(Pothos::archive::library_version_type(3) < library_version){
        ar >> POTHOS_SERIALIZATION_NVP(item_version);
    }
    Pothos::serialization::detail::stack_construct<Archive, U> u(ar, item_version);
    ar >> Pothos::serialization::make_nvp("item", u.reference());
    t.push_front(u.reference());
    POTHOS_DEDUCED_TYPENAME POTHOS_STD_EXTENSION_NAMESPACE::slist<U, Allocator>::iterator last;
    last = t.begin();
    while(--count > 0){
        Pothos::serialization::detail::stack_construct<Archive, U> 
            u(ar, file_version);
        ar >> Pothos::serialization::make_nvp("item", u.reference());
        last = t.insert_after(last, u.reference());
        ar.reset_object_address(& (*last), & u.reference());
    }
}

// split non-intrusive serialization function member into separate
// non intrusive save/load member functions
template<class Archive, class U, class Allocator>
inline void serialize(
    Archive & ar,
    POTHOS_STD_EXTENSION_NAMESPACE::slist<U, Allocator> &t,
    const unsigned int file_version
){
    Pothos::serialization::split_free(ar, t, file_version);
}

} // serialization
} // namespace boost

#include <Pothos/serialization/collection_traits.hpp>

POTHOS_SERIALIZATION_COLLECTION_TRAITS(POTHOS_STD_EXTENSION_NAMESPACE::slist)

#endif  // BOOST_HAS_SLIST
#endif  // BOOST_SERIALIZATION_SLIST_HPP
