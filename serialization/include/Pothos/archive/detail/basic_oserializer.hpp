#ifndef POTHOS_SERIALIZATION_BASIC_OSERIALIZER_HPP
#define POTHOS_SERIALIZATION_BASIC_OSERIALIZER_HPP

// MS compatible compilers support #pragma once
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// basic_oserializer.hpp: extenstion of type_info required for serialization.

// (C) Copyright 2002 Robert Ramey - http://www.rrsd.com . 
// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for updates, documentation, and revision history.

#include <cstddef> // NULL
#include <Pothos/serialization/impl/config.hpp>
#include <Pothos/serialization/impl/noncopyable.hpp>

#include <Pothos/archive/basic_archive.hpp>
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

class POTHOS_ARCHIVE_DECL(POTHOS_PP_EMPTY()) basic_oarchive;
class POTHOS_ARCHIVE_DECL(POTHOS_PP_EMPTY()) basic_pointer_oserializer;

class POTHOS_ARCHIVE_DECL(POTHOS_PP_EMPTY()) basic_oserializer : 
    public basic_serializer
{
private:
    basic_pointer_oserializer *m_bpos;
protected:
    explicit basic_oserializer(
        const Pothos::serialization::extended_type_info & type_
    );
    // account for bogus gcc warning
    #if defined(__GNUC__)
    virtual
    #endif
    ~basic_oserializer();
public:
    bool serialized_as_pointer() const {
        return m_bpos != NULL;
    }
    void set_bpos(basic_pointer_oserializer *bpos){
        m_bpos = bpos;
    }
    const basic_pointer_oserializer * get_bpos() const {
        return m_bpos;
    }
    virtual void save_object_data(
        basic_oarchive & ar, const void * x
    ) const = 0;
    // returns true if class_info should be saved
    virtual bool class_info() const = 0;
    // returns true if objects should be tracked
    virtual bool tracking(const unsigned int flags) const = 0;
    // returns class version
    virtual version_type version() const = 0;
    // returns true if this class is polymorphic
    virtual bool is_polymorphic() const = 0;
};

} // namespace detail
} // namespace serialization
} // namespace boost

#ifdef POTHOS_MSVC
#pragma warning(pop)
#endif

#include <Pothos/archive/detail/abi_suffix.hpp> // pops abi_suffix.hpp pragmas

#endif // BOOST_SERIALIZATION_BASIC_OSERIALIZER_HPP
