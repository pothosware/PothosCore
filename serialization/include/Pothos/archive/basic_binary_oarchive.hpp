#ifndef POTHOS_ARCHIVE_BASIC_BINARY_OARCHIVE_HPP
#define POTHOS_ARCHIVE_BASIC_BINARY_OARCHIVE_HPP

// MS compatible compilers support #pragma once
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// basic_binary_oarchive.hpp

// (C) Copyright 2002 Robert Ramey - http://www.rrsd.com . 
// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for updates, documentation, and revision history.

// archives stored as native binary - this should be the fastest way
// to archive the state of a group of obects.  It makes no attempt to
// convert to any canonical form.

// IN GENERAL, ARCHIVES CREATED WITH THIS CLASS WILL NOT BE READABLE
// ON PLATFORM APART FROM THE ONE THEY ARE CREATE ON

#include <Pothos/serialization/impl/assert.hpp>
#include <Pothos/serialization/impl/config.hpp>
#include <Pothos/serialization/impl/detail/workaround.hpp>
#include <Pothos/serialization/pfto.hpp>

#include <Pothos/serialization/impl/integer.hpp>
#include <Pothos/serialization/impl/integer_traits.hpp>

#include <Pothos/archive/detail/common_oarchive.hpp>
#include <Pothos/serialization/string.hpp>
#include <Pothos/serialization/collection_size_type.hpp>
#include <Pothos/serialization/item_version_type.hpp>

#include <Pothos/archive/detail/abi_prefix.hpp> // must be the last header

#ifdef POTHOS_MSVC
#  pragma warning(push)
#  pragma warning(disable : 4511 4512)
#endif

namespace Pothos {
namespace archive {

//////////////////////////////////////////////////////////////////////
// class basic_binary_oarchive - write serialized objects to a binary output stream
// note: this archive has no pretensions to portability.  Archive format
// may vary across machine architectures and compilers.  About the only
// guarentee is that an archive created with this code will be readable
// by a program built with the same tools for the same machne.  This class
// does have the virtue of buiding the smalles archive in the minimum amount
// of time.  So under some circumstances it may be he right choice.
template<class Archive>
class basic_binary_oarchive : 
    public archive::detail::common_oarchive<Archive>
{
protected:
#if POTHOS_WORKAROUND(POTHOS_MSVC, <= 1300)
public:
#elif defined(POTHOS_MSVC)
    // for some inexplicable reason insertion of "class" generates compile erro
    // on msvc 7.1
    friend detail::interface_oarchive<Archive>;
#else
    friend class detail::interface_oarchive<Archive>;
#endif
    // any datatype not specifed below will be handled by base class
    typedef detail::common_oarchive<Archive> detail_common_oarchive;
    template<class T>
    void save_override(const T & t, POTHOS_PFTO int version){
      this->detail_common_oarchive::save_override(t, static_cast<int>(version));
    }

    // include these to trap a change in binary format which
    // isn't specifically handled
    POTHOS_STATIC_ASSERT(sizeof(tracking_type) == sizeof(bool));
    // upto 32K classes
    POTHOS_STATIC_ASSERT(sizeof(class_id_type) == sizeof(int_least16_t));
    POTHOS_STATIC_ASSERT(sizeof(class_id_reference_type) == sizeof(int_least16_t));
    // upto 2G objects
    POTHOS_STATIC_ASSERT(sizeof(object_id_type) == sizeof(uint_least32_t));
    POTHOS_STATIC_ASSERT(sizeof(object_reference_type) == sizeof(uint_least32_t));

    // binary files don't include the optional information 
    void save_override(const class_id_optional_type & /* t */, int){}

    // enable this if we decide to support generation of previous versions
    #if 0
    void save_override(const Pothos::archive::version_type & t, int version){
        library_version_type lvt = this->get_library_version();
        if(Pothos::archive::library_version_type(7) < lvt){
            this->detail_common_oarchive::save_override(t, version);
        }
        else
        if(Pothos::archive::library_version_type(6) < lvt){
            const Pothos::uint_least16_t x = t;
            * this->This() << x;
        }
        else{
            const unsigned int x = t;
            * this->This() << x;
        }
    }
    void save_override(const Pothos::serialization::item_version_type & t, int version){
        library_version_type lvt = this->get_library_version();
        if(Pothos::archive::library_version_type(7) < lvt){
            this->detail_common_oarchive::save_override(t, version);
        }
        else
        if(Pothos::archive::library_version_type(6) < lvt){
            const Pothos::uint_least16_t x = t;
            * this->This() << x;
        }
        else{
            const unsigned int x = t;
            * this->This() << x;
        }
    }

    void save_override(class_id_type & t, int version){
        library_version_type lvt = this->get_library_version();
        if(Pothos::archive::library_version_type(7) < lvt){
            this->detail_common_oarchive::save_override(t, version);
        }
        else
        if(Pothos::archive::library_version_type(6) < lvt){
            const Pothos::int_least16_t x = t;
            * this->This() << x;
        }
        else{
            const int x = t;
            * this->This() << x;
        }
    }
    void save_override(class_id_reference_type & t, int version){
        save_override(static_cast<class_id_type &>(t), version);
    }

    #endif

    // explicitly convert to char * to avoid compile ambiguities
    void save_override(const class_name_type & t, int){
        const std::string s(t);
        * this->This() << s;
    }

    #if 0
    void save_override(const serialization::collection_size_type & t, int){
        if (get_library_version() < Pothos::archive::library_version_type(6)){
            unsigned int x=0;
            * this->This() >> x;
            t = serialization::collection_size_type(x);
        } 
        else{
            * this->This() >> t;
        }
    }
    #endif
    POTHOS_ARCHIVE_OR_WARCHIVE_DECL(void)
    init();

    basic_binary_oarchive(unsigned int flags) :
        detail::common_oarchive<Archive>(flags)
    {}
};

} // namespace archive
} // namespace boost

#ifdef POTHOS_MSVC
#pragma warning(pop)
#endif

#include <Pothos/archive/detail/abi_suffix.hpp> // pops abi_suffix.hpp pragmas

#endif // BOOST_ARCHIVE_BASIC_BINARY_OARCHIVE_HPP
