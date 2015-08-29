#ifndef POTHOS_ARCHIVE_BASIC_TEXT_OPRIMITIVE_HPP
#define POTHOS_ARCHIVE_BASIC_TEXT_OPRIMITIVE_HPP

// MS compatible compilers support #pragma once
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// basic_text_oprimitive.hpp

// (C) Copyright 2002 Robert Ramey - http://www.rrsd.com . 
// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for updates, documentation, and revision history.

// archives stored as text - note these ar templated on the basic
// stream templates to accommodate wide (and other?) kind of characters
//
// note the fact that on libraries without wide characters, ostream is
// is not a specialization of basic_ostream which in fact is not defined
// in such cases.   So we can't use basic_ostream<OStream::char_type> but rather
// use two template parameters

#include <iomanip>
#include <locale>
#include <Pothos/serialization/impl/config/no_tr1/cmath.hpp> // isnan
#include <Pothos/serialization/impl/assert.hpp>
#include <cstddef> // size_t

#include <Pothos/serialization/impl/config.hpp>
#include <Pothos/serialization/impl/static_assert.hpp>
#include <Pothos/serialization/impl/detail/workaround.hpp>
#if POTHOS_WORKAROUND(POTHOS_DINKUMWARE_STDLIB, == 1)
#include <Pothos/archive/dinkumware.hpp>
#endif

#if defined(POTHOS_NO_STDC_NAMESPACE)
namespace std{ 
    using ::size_t;
    #if ! defined(POTHOS_DINKUMWARE_STDLIB) && ! defined(__SGI_STL_PORT)
        using ::locale;
    #endif
} // namespace std
#endif

#include <Pothos/serialization/impl/limits.hpp>
#include <Pothos/serialization/impl/integer.hpp>
#include <Pothos/serialization/impl/io/ios_state.hpp>
#include <Pothos/serialization/impl/scoped_ptr.hpp>
#include <Pothos/serialization/throw_exception.hpp>
#include <Pothos/archive/archive_exception.hpp>
#include <Pothos/archive/basic_streambuf_locale_saver.hpp>
#include <Pothos/archive/detail/abi_prefix.hpp> // must be the last header

namespace Pothos {
namespace archive {

class save_access;

/////////////////////////////////////////////////////////////////////////
// class basic_text_oprimitive - output of prmitives to stream
template<class OStream>
class basic_text_oprimitive
{
#ifndef POTHOS_NO_MEMBER_TEMPLATE_FRIENDS
protected:
#else
public:
#endif
    OStream &os;
    io::ios_flags_saver flags_saver;
    io::ios_precision_saver precision_saver;

    #ifndef POTHOS_NO_STD_LOCALE
    Pothos::scoped_ptr<std::locale> archive_locale;
    basic_streambuf_locale_saver<
        POTHOS_DEDUCED_TYPENAME OStream::char_type, 
        POTHOS_DEDUCED_TYPENAME OStream::traits_type
    > locale_saver;
    #endif

    // default saving of primitives.
    template<class T>
    void save(const T &t){
        if(os.fail())
            Pothos::serialization::throw_exception(
                archive_exception(archive_exception::output_stream_error)
            );
        os << t;
    }

    /////////////////////////////////////////////////////////
    // fundamental types that need special treatment
    void save(const bool t){
        // trap usage of invalid uninitialized boolean which would
        // otherwise crash on load.
        POTHOS_ASSERT(0 == static_cast<int>(t) || 1 == static_cast<int>(t));
        if(os.fail())
            Pothos::serialization::throw_exception(
                archive_exception(archive_exception::output_stream_error)
            );
        os << t;
    }
    void save(const signed char t)
    {
        save(static_cast<short int>(t));
    }
    void save(const unsigned char t)
    {
        save(static_cast<short unsigned int>(t));
    }
    void save(const char t)
    {
        save(static_cast<short int>(t));
    }
    #ifndef POTHOS_NO_INTRINSIC_WCHAR_T
    void save(const wchar_t t)
    {
        POTHOS_STATIC_ASSERT(sizeof(wchar_t) <= sizeof(int));
        save(static_cast<int>(t));
    }
    #endif
    void save(const float t)
    {
        // must be a user mistake - can't serialize un-initialized data
        if(os.fail())
            Pothos::serialization::throw_exception(
                archive_exception(archive_exception::output_stream_error)
            );
        os << std::setprecision(std::numeric_limits<float>::digits10 + 2);
        os << t;
    }
    void save(const double t)
    {
        // must be a user mistake - can't serialize un-initialized data
        if(os.fail())
            Pothos::serialization::throw_exception(
                archive_exception(archive_exception::output_stream_error)
            );
        os << std::setprecision(std::numeric_limits<double>::digits10 + 2);
        os << t;
    }
    POTHOS_ARCHIVE_OR_WARCHIVE_DECL(POTHOS_PP_EMPTY())
    basic_text_oprimitive(OStream & os, bool no_codecvt);
    POTHOS_ARCHIVE_OR_WARCHIVE_DECL(POTHOS_PP_EMPTY()) 
    ~basic_text_oprimitive();
public:
    // unformatted append of one character
    void put(POTHOS_DEDUCED_TYPENAME OStream::char_type c){
        if(os.fail())
            Pothos::serialization::throw_exception(
                archive_exception(archive_exception::output_stream_error)
            );
        os.put(c);
    }
    // unformatted append of null terminated string
    void put(const char * s){
        while('\0' != *s)
            os.put(*s++);
    }
    POTHOS_ARCHIVE_OR_WARCHIVE_DECL(void) 
    save_binary(const void *address, std::size_t count);
};

} //namespace boost 
} //namespace archive 

#include <Pothos/archive/detail/abi_suffix.hpp> // pops abi_suffix.hpp pragmas

#endif // BOOST_ARCHIVE_BASIC_TEXT_OPRIMITIVE_HPP
