/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// basic_text_iprimitive.ipp:

// (C) Copyright 2002 Robert Ramey - http://www.rrsd.com .
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for updates, documentation, and revision history.

#include <cstddef> // size_t
#include <cstddef> // NULL

#include <Pothos/serialization/impl/config.hpp>
#if defined(POTHOS_NO_STDC_NAMESPACE)
namespace std{ 
    using ::size_t; 
} // namespace std
#endif

#include <Pothos/serialization/throw_exception.hpp>
#include <Pothos/serialization/pfto.hpp>

#include <Pothos/archive/basic_text_iprimitive.hpp>
#include <Pothos/archive/codecvt_null.hpp>
#include <Pothos/archive/add_facet.hpp>

#include <Pothos/archive/iterators/remove_whitespace.hpp>
#include <Pothos/archive/iterators/istream_iterator.hpp>
#include <Pothos/archive/iterators/binary_from_base64.hpp>
#include <Pothos/archive/iterators/transform_width.hpp>

namespace Pothos { 
namespace archive {

namespace {
    template<class CharType>
    bool is_whitespace(CharType c);

    template<>
    bool is_whitespace(char t){
        return 0 != std::isspace(t);
    }

    #ifndef POTHOS_NO_CWCHAR
    template<>
    bool is_whitespace(wchar_t t){
        return 0 != std::iswspace(t);
    }
    #endif
}

// translate base64 text into binary and copy into buffer
// until buffer is full.
template<class IStream>
POTHOS_ARCHIVE_OR_WARCHIVE_DECL(void)
basic_text_iprimitive<IStream>::load_binary(
    void *address, 
    std::size_t count
){
    typedef POTHOS_DEDUCED_TYPENAME IStream::char_type CharType;
    
    if(0 == count)
        return;
        
    POTHOS_ASSERT(
        static_cast<std::size_t>((std::numeric_limits<std::streamsize>::max)())
        > (count + sizeof(CharType) - 1)/sizeof(CharType)
    );
        
    if(is.fail())
        Pothos::serialization::throw_exception(
            archive_exception(archive_exception::input_stream_error)
        );
    // convert from base64 to binary
    typedef POTHOS_DEDUCED_TYPENAME
        iterators::transform_width<
            iterators::binary_from_base64<
                iterators::remove_whitespace<
                    iterators::istream_iterator<CharType>
                >
                ,CharType
            >
            ,8
            ,6
            ,CharType
        > 
        binary;
        
    binary i = binary(
        POTHOS_MAKE_PFTO_WRAPPER(
            iterators::istream_iterator<CharType>(is)
        )
    );

    char * caddr = static_cast<char *>(address);
    
    // take care that we don't increment anymore than necessary
    while(count-- > 0){
        *caddr++ = static_cast<char>(*i++);
    }

    // skip over any excess input
    for(;;){
        POTHOS_DEDUCED_TYPENAME IStream::int_type r;
        r = is.get();
        if(is.eof())
            break;
        if(is_whitespace(static_cast<CharType>(r)))
            break;
    }
}
    
template<class IStream>
POTHOS_ARCHIVE_OR_WARCHIVE_DECL(POTHOS_PP_EMPTY())
basic_text_iprimitive<IStream>::basic_text_iprimitive(
    IStream  &is_,
    bool no_codecvt
) :
#ifndef POTHOS_NO_STD_LOCALE
    is(is_),
    flags_saver(is_),
    precision_saver(is_),
    archive_locale(NULL),
    locale_saver(* is_.rdbuf())
{
    if(! no_codecvt){
        archive_locale.reset(
            add_facet(
                std::locale::classic(), 
                new codecvt_null<POTHOS_DEDUCED_TYPENAME IStream::char_type>
            )
        );
        is.imbue(* archive_locale);
    }
    is >> std::noboolalpha;
}
#else
    is(is_),
    flags_saver(is_),
    precision_saver(is_)
{}
#endif

template<class IStream>
POTHOS_ARCHIVE_OR_WARCHIVE_DECL(POTHOS_PP_EMPTY())
basic_text_iprimitive<IStream>::~basic_text_iprimitive(){
    is.sync();
}

} // namespace archive
} // namespace boost
