#ifndef POTHOS_ARCHIVE_BINARY_IARCHIVE_IMPL_HPP
#define POTHOS_ARCHIVE_BINARY_IARCHIVE_IMPL_HPP

// MS compatible compilers support #pragma once
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// binary_iarchive_impl.hpp

// (C) Copyright 2002 Robert Ramey - http://www.rrsd.com . 
// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for updates, documentation, and revision history.

#include <istream>
#include <Pothos/serialization/pfto.hpp>
#include <Pothos/archive/basic_binary_iprimitive.hpp>
#include <Pothos/archive/basic_binary_iarchive.hpp>

#ifdef POTHOS_MSVC
#  pragma warning(push)
#  pragma warning(disable : 4511 4512)
#endif

namespace Pothos { 
namespace archive {

template<class Archive, class Elem, class Tr>
class binary_iarchive_impl : 
    public basic_binary_iprimitive<Archive, Elem, Tr>,
    public basic_binary_iarchive<Archive>
{
#ifdef POTHOS_NO_MEMBER_TEMPLATE_FRIENDS
public:
#else
    friend class detail::interface_iarchive<Archive>;
    friend class basic_binary_iarchive<Archive>;
    friend class load_access;
protected:
#endif
    // note: the following should not needed - but one compiler (vc 7.1)
    // fails to compile one test (test_shared_ptr) without it !!!
    // make this protected so it can be called from a derived archive
    template<class T>
    void load_override(T & t, POTHOS_PFTO int){
        this->basic_binary_iarchive<Archive>::load_override(t, 0L);
    }
    void init(unsigned int flags){
        if(0 != (flags & no_header))
            return;
        #if ! defined(__MWERKS__)
            this->basic_binary_iarchive<Archive>::init();
            this->basic_binary_iprimitive<Archive, Elem, Tr>::init();
        #else
            basic_binary_iarchive<Archive>::init();
            basic_binary_iprimitive<Archive, Elem, Tr>::init();
        #endif
    }
    binary_iarchive_impl(
        std::basic_streambuf<Elem, Tr> & bsb, 
        unsigned int flags
    ) :
        basic_binary_iprimitive<Archive, Elem, Tr>(
            bsb, 
            0 != (flags & no_codecvt)
        ),
        basic_binary_iarchive<Archive>(flags)
    {
        init(flags);
    }
    binary_iarchive_impl(
        std::basic_istream<Elem, Tr> & is, 
        unsigned int flags
    ) :
        basic_binary_iprimitive<Archive, Elem, Tr>(
            * is.rdbuf(), 
            0 != (flags & no_codecvt)
        ),
        basic_binary_iarchive<Archive>(flags)
    {
        init(flags);
    }
};

} // namespace archive
} // namespace boost

#ifdef POTHOS_MSVC
#pragma warning(pop)
#endif

#endif // BOOST_ARCHIVE_BINARY_IARCHIVE_IMPL_HPP
