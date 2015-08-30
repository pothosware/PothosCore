#ifndef POTHOS_ARCHIVE_BASIC_POINTER_ISERIALIZER_HPP
#define POTHOS_ARCHIVE_BASIC_POINTER_ISERIALIZER_HPP

// MS compatible compilers support #pragma once
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// basic_pointer_oserializer.hpp: extenstion of type_info required for 
// serialization.

// (C) Copyright 2002 Robert Ramey - http://www.rrsd.com . 
// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for updates, documentation, and revision history.
#include <Pothos/serialization/impl/config.hpp>
#include <Pothos/serialization/impl/noncopyable.hpp>
#include <Pothos/archive/detail/auto_link_archive.hpp>
#include <Pothos/archive/detail/basic_serializer.hpp>

#include <Pothos/archive/detail/abi_prefix.hpp> // must be the last header

#ifdef POTHOS_MSVC
#  pragma warning(push)
#  pragma warning(disable : 4511 4512)
#endif

namespace Pothos {
namespace serialization {
    class extended_type_info;
} // namespace serialization

// forward declarations
namespace archive {
namespace detail {

class POTHOS_ARCHIVE_DECL(POTHOS_PP_EMPTY()) basic_iarchive;
class POTHOS_ARCHIVE_DECL(POTHOS_PP_EMPTY()) basic_iserializer;

class POTHOS_ARCHIVE_DECL(POTHOS_PP_EMPTY()) basic_pointer_iserializer 
    : public basic_serializer {
protected:
    explicit basic_pointer_iserializer(
        const Pothos::serialization::extended_type_info & type_
    );
    // account for bogus gcc warning
    #if defined(__GNUC__)
    virtual
    #endif
    ~basic_pointer_iserializer();
public:
    virtual const basic_iserializer & get_basic_serializer() const = 0;
    virtual void load_object_ptr(
        basic_iarchive & ar, 
        void * & x,
        const unsigned int file_version
    ) const = 0;
};

} // namespace detail
} // namespace archive
} // namespace boost

#ifdef POTHOS_MSVC
#pragma warning(pop)
#endif

#include <Pothos/archive/detail/abi_suffix.hpp> // pops abi_suffix.hpp pragmas

#endif // BOOST_ARCHIVE_BASIC_POINTER_ISERIALIZER_HPP
