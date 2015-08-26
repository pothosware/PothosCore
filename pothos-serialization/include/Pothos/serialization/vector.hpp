#ifndef  POTHOS_SERIALIZATION_VECTOR_HPP
#define POTHOS_SERIALIZATION_VECTOR_HPP

// MS compatible compilers support #pragma once
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// vector.hpp: serialization for stl vector templates

// (C) Copyright 2002 Robert Ramey - http://www.rrsd.com . 
// fast array serialization (C) Copyright 2005 Matthias Troyer 
// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for updates, documentation, and revision history.

#include <vector>

#include <Pothos/serialization/impl/config.hpp>
#include <Pothos/serialization/impl/detail/workaround.hpp>
#include <Pothos/serialization/impl/type_traits/is_arithmetic.hpp> 

#include <Pothos/serialization/collections_save_imp.hpp>
#include <Pothos/serialization/collections_load_imp.hpp>
#include <Pothos/serialization/split_free.hpp>
#include <Pothos/serialization/array.hpp>
#include <Pothos/serialization/detail/get_data.hpp>
#include <Pothos/serialization/impl/mpl/bool.hpp>

// default is being compatible with version 1.34.1 files, not 1.35 files
#ifndef POTHOS_SERIALIZATION_VECTOR_VERSIONED
#define POTHOS_SERIALIZATION_VECTOR_VERSIONED(V) (V==4 || V==5)
#endif

// function specializations must be defined in the appropriate
// namespace - boost::serialization
#if defined(__SGI_STL_PORT) || defined(_STLPORT_VERSION)
#define STD _STLP_STD
#else
#define STD std
#endif

namespace Pothos { 
namespace serialization {

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// vector< T >

// the default versions

template<class Archive, class U, class Allocator>
inline void save(
    Archive & ar,
    const std::vector<U, Allocator> &t,
    const unsigned int /* file_version */,
    mpl::false_
){
    Pothos::serialization::stl::save_collection<Archive, STD::vector<U, Allocator> >(
        ar, t
    );
}

template<class Archive, class U, class Allocator>
inline void load(
    Archive & ar,
    std::vector<U, Allocator> &t,
    const unsigned int /* file_version */,
    mpl::false_
){
    Pothos::serialization::stl::load_collection<
        Archive,
        std::vector<U, Allocator>,
        Pothos::serialization::stl::archive_input_seq<
            Archive, STD::vector<U, Allocator> 
        >,
        Pothos::serialization::stl::reserve_imp<STD::vector<U, Allocator> >
    >(ar, t);
}

// the optimized versions

template<class Archive, class U, class Allocator>
inline void save(
    Archive & ar,
    const std::vector<U, Allocator> &t,
    const unsigned int /* file_version */,
    mpl::true_
){
    const collection_size_type count(t.size());
    ar << POTHOS_SERIALIZATION_NVP(count);
    if (!t.empty())
        ar << make_array(detail::get_data(t),t.size());
}

template<class Archive, class U, class Allocator>
inline void load(
    Archive & ar,
    std::vector<U, Allocator> &t,
    const unsigned int /* file_version */,
    mpl::true_
){
    collection_size_type count(t.size());
    ar >> POTHOS_SERIALIZATION_NVP(count);
    t.resize(count);
    unsigned int item_version=0;
    if(POTHOS_SERIALIZATION_VECTOR_VERSIONED(ar.get_library_version())) {
        ar >> POTHOS_SERIALIZATION_NVP(item_version);
    }
    if (!t.empty())
        ar >> make_array(detail::get_data(t),t.size());
  }

// dispatch to either default or optimized versions

template<class Archive, class U, class Allocator>
inline void save(
    Archive & ar,
    const std::vector<U, Allocator> &t,
    const unsigned int file_version
){
    typedef POTHOS_DEDUCED_TYPENAME 
    Pothos::serialization::use_array_optimization<Archive>::template apply<
        POTHOS_DEDUCED_TYPENAME remove_const<U>::type 
    >::type use_optimized;
    save(ar,t,file_version, use_optimized());
}

template<class Archive, class U, class Allocator>
inline void load(
    Archive & ar,
    std::vector<U, Allocator> &t,
    const unsigned int file_version
){
#ifdef POTHOS_SERIALIZATION_VECTOR_135_HPP
    if (ar.get_library_version()==Pothos::archive::library_version_type(5))
    {
      load(ar,t,file_version, Pothos::is_arithmetic<U>());
      return;
    }
#endif
    typedef POTHOS_DEDUCED_TYPENAME 
    Pothos::serialization::use_array_optimization<Archive>::template apply<
        POTHOS_DEDUCED_TYPENAME remove_const<U>::type 
    >::type use_optimized;
    load(ar,t,file_version, use_optimized());
}

// split non-intrusive serialization function member into separate
// non intrusive save/load member functions
template<class Archive, class U, class Allocator>
inline void serialize(
    Archive & ar,
    std::vector<U, Allocator> & t,
    const unsigned int file_version
){
    Pothos::serialization::split_free(ar, t, file_version);
}

#if ! POTHOS_WORKAROUND(POTHOS_MSVC, <= 1300)

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// vector<bool>
template<class Archive, class Allocator>
inline void save(
    Archive & ar,
    const std::vector<bool, Allocator> &t,
    const unsigned int /* file_version */
){
    // record number of elements
    collection_size_type count (t.size());
    ar << POTHOS_SERIALIZATION_NVP(count);
    std::vector<bool>::const_iterator it = t.begin();
    while(count-- > 0){
        bool tb = *it++;
        ar << Pothos::serialization::make_nvp("item", tb);
    }
}

template<class Archive, class Allocator>
inline void load(
    Archive & ar,
    std::vector<bool, Allocator> &t,
    const unsigned int /* file_version */
){
    // retrieve number of elements
    collection_size_type count;
    ar >> POTHOS_SERIALIZATION_NVP(count);
    t.clear();
    while(count-- > 0){
        bool i;
        ar >> Pothos::serialization::make_nvp("item", i);
        t.push_back(i);
    }
}

// split non-intrusive serialization function member into separate
// non intrusive save/load member functions
template<class Archive, class Allocator>
inline void serialize(
    Archive & ar,
    std::vector<bool, Allocator> & t,
    const unsigned int file_version
){
    Pothos::serialization::split_free(ar, t, file_version);
}

#endif // BOOST_WORKAROUND

} // serialization
} // namespace boost

#include <Pothos/serialization/collection_traits.hpp>

POTHOS_SERIALIZATION_COLLECTION_TRAITS(std::vector)
#undef STD

#endif // BOOST_SERIALIZATION_VECTOR_HPP
