#ifndef POTHOS_ARCHIVE_BINARY_IPRIMITIVE_HPP
#define POTHOS_ARCHIVE_BINARY_IPRIMITIVE_HPP

// MS compatible compilers support #pragma once
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#if defined(_MSC_VER)
#pragma warning( disable : 4800 )
#endif

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// basic_binary_iprimitive.hpp
//
// archives stored as native binary - this should be the fastest way
// to archive the state of a group of obects.  It makes no attempt to
// convert to any canonical form.

// IN GENERAL, ARCHIVES CREATED WITH THIS CLASS WILL NOT BE READABLE
// ON PLATFORM APART FROM THE ONE THEY ARE CREATED ON

// (C) Copyright 2002 Robert Ramey - http://www.rrsd.com . 
// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for updates, documentation, and revision history.

#include <iosfwd>
#include <Pothos/serialization/impl/assert.hpp>
#include <locale>
#include <cstring> // std::memcpy
#include <cstddef> // std::size_t
#include <streambuf> // basic_streambuf
#include <string>

#include <Pothos/serialization/impl/config.hpp>
#if defined(POTHOS_NO_STDC_NAMESPACE)
namespace std{ 
    using ::memcpy; 
    using ::size_t;
} // namespace std
#endif

#include <Pothos/serialization/impl/cstdint.hpp>
#include <Pothos/serialization/impl/scoped_ptr.hpp>
#include <Pothos/serialization/throw_exception.hpp>
#include <Pothos/serialization/impl/integer.hpp>
#include <Pothos/serialization/impl/integer_traits.hpp>

#include <Pothos/archive/basic_streambuf_locale_saver.hpp>
#include <Pothos/archive/archive_exception.hpp>
#include <Pothos/serialization/impl/mpl/placeholders.hpp>
#include <Pothos/serialization/is_bitwise_serializable.hpp>
#include <Pothos/serialization/array.hpp>
#include <Pothos/archive/detail/auto_link_archive.hpp>
#include <Pothos/archive/detail/abi_prefix.hpp> // must be the last header

namespace Pothos { 
namespace archive {

/////////////////////////////////////////////////////////////////////////////
// class binary_iarchive - read serialized objects from a input binary stream
template<class Archive, class Elem, class Tr>
class basic_binary_iprimitive
{
#ifndef POTHOS_NO_MEMBER_TEMPLATE_FRIENDS
    friend class load_access;
protected:
#else
public:
#endif
    std::basic_streambuf<Elem, Tr> & m_sb;
    // return a pointer to the most derived class
    Archive * This(){
        return static_cast<Archive *>(this);
    }

    #ifndef POTHOS_NO_STD_LOCALE
    Pothos::scoped_ptr<std::locale> archive_locale;
    basic_streambuf_locale_saver<Elem, Tr> locale_saver;
    #endif

    // main template for serilization of primitive types
    template<class T>
    void load(T & t){
        load_binary(& t, sizeof(T));
    }

    /////////////////////////////////////////////////////////
    // fundamental types that need special treatment
    
    // trap usage of invalid uninitialized boolean 
    void load(bool & t){
        load_binary(& t, sizeof(t));
        int i = t;
        POTHOS_ASSERT(0 == i || 1 == i);
        (void)i; // warning suppression for release builds.
    }
    POTHOS_ARCHIVE_OR_WARCHIVE_DECL(void)
    load(std::string &s);
    #ifndef POTHOS_NO_STD_WSTRING
    POTHOS_ARCHIVE_OR_WARCHIVE_DECL(void)
    load(std::wstring &ws);
    #endif
    POTHOS_ARCHIVE_OR_WARCHIVE_DECL(void)
    load(char * t);
    POTHOS_ARCHIVE_OR_WARCHIVE_DECL(void)
    load(wchar_t * t);

    POTHOS_ARCHIVE_OR_WARCHIVE_DECL(void)
    init();
    POTHOS_ARCHIVE_OR_WARCHIVE_DECL(POTHOS_PP_EMPTY()) 
    basic_binary_iprimitive(
        std::basic_streambuf<Elem, Tr> & sb, 
        bool no_codecvt
    );
    POTHOS_ARCHIVE_OR_WARCHIVE_DECL(POTHOS_PP_EMPTY()) 
    ~basic_binary_iprimitive();
public:
    // we provide an optimized load for all fundamental types
    // typedef serialization::is_bitwise_serializable<mpl::_1> 
    // use_array_optimization;
    struct use_array_optimization {  
        template <class T>  
        #if defined(POTHOS_NO_DEPENDENT_NESTED_DERIVATIONS)  
            struct apply {  
                typedef POTHOS_DEDUCED_TYPENAME Pothos::serialization::is_bitwise_serializable< T >::type type;  
            };
        #else
            struct apply : public Pothos::serialization::is_bitwise_serializable< T > {};  
        #endif
    };

    // the optimized load_array dispatches to load_binary 
    template <class ValueType>
    void load_array(serialization::array<ValueType>& a, unsigned int)
    {
      load_binary(a.address(),a.count()*sizeof(ValueType));
    }

    void
    load_binary(void *address, std::size_t count);
};

template<class Archive, class Elem, class Tr>
inline void
basic_binary_iprimitive<Archive, Elem, Tr>::load_binary(
    void *address, 
    std::size_t count
){
    // note: an optimizer should eliminate the following for char files
    POTHOS_ASSERT(
        static_cast<std::streamsize>(count / sizeof(Elem)) 
        <= Pothos::integer_traits<std::streamsize>::const_max
    );
    std::streamsize s = static_cast<std::streamsize>(count / sizeof(Elem));
    std::streamsize scount = m_sb.sgetn(
        static_cast<Elem *>(address), 
        s
    );
    if(scount != s)
        Pothos::serialization::throw_exception(
            archive_exception(archive_exception::input_stream_error)
        );
    // note: an optimizer should eliminate the following for char files
    POTHOS_ASSERT(count % sizeof(Elem) <= Pothos::integer_traits<std::streamsize>::const_max);
    s = static_cast<std::streamsize>(count % sizeof(Elem));
    if(0 < s){
//        if(is.fail())
//            boost::serialization::throw_exception(
//                archive_exception(archive_exception::stream_error)
//        );
        Elem t;
        scount = m_sb.sgetn(& t, 1);
        if(scount != 1)
            Pothos::serialization::throw_exception(
                archive_exception(archive_exception::input_stream_error)
            );
        std::memcpy(static_cast<char*>(address) + (count - s), &t, s);
    }
}

} // namespace archive
} // namespace boost

#include <Pothos/archive/detail/abi_suffix.hpp> // pop pragmas

#endif // BOOST_ARCHIVE_BINARY_IPRIMITIVE_HPP
