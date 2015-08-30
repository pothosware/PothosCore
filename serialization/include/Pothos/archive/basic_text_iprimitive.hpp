#ifndef POTHOS_ARCHIVE_BASIC_TEXT_IPRIMITIVE_HPP
#define POTHOS_ARCHIVE_BASIC_TEXT_IPRIMITIVE_HPP

// MS compatible compilers support #pragma once
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// basic_text_iprimitive.hpp

// (C) Copyright 2002 Robert Ramey - http://www.rrsd.com . 
// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for updates, documentation, and revision history.

// archives stored as text - note these are templated on the basic
// stream templates to accommodate wide (and other?) kind of characters
//
// Note the fact that on libraries without wide characters, ostream is
// not a specialization of basic_ostream which in fact is not defined
// in such cases.   So we can't use basic_ostream<IStream::char_type> but rather
// use two template parameters

#include <Pothos/serialization/impl/assert.hpp>
#include <locale>
#include <cstddef> // size_t

#include <Pothos/serialization/impl/config.hpp>
#if defined(POTHOS_NO_STDC_NAMESPACE)
namespace std{ 
    using ::size_t; 
    #if ! defined(POTHOS_DINKUMWARE_STDLIB) && ! defined(__SGI_STL_PORT)
        using ::locale;
    #endif
} // namespace std
#endif

#include <Pothos/serialization/impl/detail/workaround.hpp>
#if POTHOS_WORKAROUND(POTHOS_DINKUMWARE_STDLIB, == 1)
#include <Pothos/archive/dinkumware.hpp>
#endif

#include <Pothos/serialization/impl/limits.hpp>
#include <Pothos/serialization/impl/io/ios_state.hpp>
#include <Pothos/serialization/impl/scoped_ptr.hpp>
#include <Pothos/serialization/impl/static_assert.hpp>

#include <Pothos/serialization/throw_exception.hpp>
#include <Pothos/archive/archive_exception.hpp>
#include <Pothos/archive/basic_streambuf_locale_saver.hpp>
#include <Pothos/archive/detail/abi_prefix.hpp> // must be the last header

namespace Pothos {
namespace archive {

/////////////////////////////////////////////////////////////////////////
// class basic_text_iarchive - load serialized objects from a input text stream
#if defined(_MSC_VER)
#pragma warning( push )
#pragma warning( disable : 4244 4267 )
#endif

template<class IStream>
class basic_text_iprimitive
{
#ifndef POTHOS_NO_MEMBER_TEMPLATE_FRIENDS
protected:
#else
public:
#endif
    IStream &is;
    io::ios_flags_saver flags_saver;
    io::ios_precision_saver precision_saver;

    #ifndef POTHOS_NO_STD_LOCALE
    Pothos::scoped_ptr<std::locale> archive_locale;
    basic_streambuf_locale_saver<
        POTHOS_DEDUCED_TYPENAME IStream::char_type, 
        POTHOS_DEDUCED_TYPENAME IStream::traits_type
    > locale_saver;
    #endif

    template<class T>
    void load(T & t)
    {
        if(! is.fail()){
            is >> t;
            return;
        }
        Pothos::serialization::throw_exception(
            archive_exception(archive_exception::input_stream_error)
        );
    }

    void load(char & t)
    {
        short int i;
        load(i);
        t = i;
    }
    void load(signed char & t)
    {
        short int i;
        load(i);
        t = i;
    }
    void load(unsigned char & t)
    {
        unsigned short int i;
        load(i);
        t = i;
    }

    #ifndef POTHOS_NO_INTRINSIC_WCHAR_T
    void load(wchar_t & t)
    {
        POTHOS_STATIC_ASSERT(sizeof(wchar_t) <= sizeof(int));
        int i;
        load(i);
        t = i;
    }
    #endif
    POTHOS_ARCHIVE_OR_WARCHIVE_DECL(POTHOS_PP_EMPTY()) 
    basic_text_iprimitive(IStream  &is, bool no_codecvt);
    POTHOS_ARCHIVE_OR_WARCHIVE_DECL(POTHOS_PP_EMPTY()) 
    ~basic_text_iprimitive();
public:
    POTHOS_ARCHIVE_OR_WARCHIVE_DECL(void)
    load_binary(void *address, std::size_t count);
};

#if defined(_MSC_VER)
#pragma warning( pop )
#endif

} // namespace archive
} // namespace boost

#include <Pothos/archive/detail/abi_suffix.hpp> // pop pragmas

#endif // BOOST_ARCHIVE_BASIC_TEXT_IPRIMITIVE_HPP
