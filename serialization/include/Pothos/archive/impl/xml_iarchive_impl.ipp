/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// xml_iarchive_impl.cpp:

// (C) Copyright 2002 Robert Ramey - http://www.rrsd.com .
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for updates, documentation, and revision history.

#include <Pothos/serialization/impl/config.hpp>
#include <cstring> // memcpy
#include <cstddef> // NULL
#if defined(POTHOS_NO_STDC_NAMESPACE)
namespace std{ 
    using ::memcpy;
} // namespace std
#endif

#ifndef POTHOS_NO_CWCHAR
#include <cstdlib> // mbtowc
#if defined(POTHOS_NO_STDC_NAMESPACE)
namespace std{ 
    using ::mbtowc;
 } // namespace std
#endif
#endif // BOOST_NO_CWCHAR

#include <Pothos/serialization/impl/detail/workaround.hpp> // RogueWave and Dinkumware
#if POTHOS_WORKAROUND(POTHOS_DINKUMWARE_STDLIB, == 1)
#include <Pothos/archive/dinkumware.hpp>
#endif

#include <Pothos/serialization/impl/detail/no_exceptions_support.hpp>

#include <Pothos/archive/xml_archive_exception.hpp>
#include <Pothos/archive/iterators/dataflow_exception.hpp>
#include <Pothos/archive/basic_xml_archive.hpp>
#include <Pothos/archive/xml_iarchive.hpp>

#include "basic_xml_grammar.hpp"

namespace Pothos {
namespace archive {

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// implemenations of functions specific to char archives

// wide char stuff used by char archives

#ifndef POTHOS_NO_CWCHAR
#ifndef POTHOS_NO_STD_WSTRING
template<class Archive>
POTHOS_ARCHIVE_DECL(void)
xml_iarchive_impl<Archive>::load(std::wstring &ws){
    std::string s;
    bool result = gimpl->parse_string(is, s);
    if(! result)
        Pothos::serialization::throw_exception(
            xml_archive_exception(xml_archive_exception::xml_archive_parsing_error)
        );
    
    #if POTHOS_WORKAROUND(_RWSTD_VER, POTHOS_TESTED_AT(20101))
    if(NULL != ws.data())
    #endif
        ws.resize(0);
    const char * start = s.data();
    const char * end = start + s.size();
    while(start < end){
        wchar_t wc;
        int resultx = std::mbtowc(&wc, start, end - start);
        if(0 < resultx){
            start += resultx;
            ws += wc;
            continue;
        }
        Pothos::serialization::throw_exception(
            iterators::dataflow_exception(
                iterators::dataflow_exception::invalid_conversion
            )
        );
    }
}
#endif // BOOST_NO_STD_WSTRING

#ifndef POTHOS_NO_INTRINSIC_WCHAR_T
template<class Archive>
POTHOS_ARCHIVE_DECL(void)
xml_iarchive_impl<Archive>::load(wchar_t * ws){
    std::string s;
    bool result = gimpl->parse_string(is, s);
    if(! result)
        Pothos::serialization::throw_exception(
            xml_archive_exception(xml_archive_exception::xml_archive_parsing_error)
        );
        
    const char * start = s.data();
    const char * end = start + s.size();
    while(start < end){
        wchar_t wc;
        int result = std::mbtowc(&wc, start, end - start);
        if(0 < result){
            start += result;
            *ws++ = wc;
            continue;
        }
        Pothos::serialization::throw_exception(
            iterators::dataflow_exception(
                iterators::dataflow_exception::invalid_conversion
            )
        );
    }
    *ws = L'\0';
}
#endif // BOOST_NO_INTRINSIC_WCHAR_T

#endif // BOOST_NO_CWCHAR

template<class Archive>
POTHOS_ARCHIVE_DECL(void)
xml_iarchive_impl<Archive>::load(std::string &s){
    bool result = gimpl->parse_string(is, s);
    if(! result)
        Pothos::serialization::throw_exception(
            xml_archive_exception(xml_archive_exception::xml_archive_parsing_error)
        );
}

template<class Archive>
POTHOS_ARCHIVE_DECL(void)
xml_iarchive_impl<Archive>::load(char * s){
    std::string tstring;
    bool result = gimpl->parse_string(is, tstring);
    if(! result)
        Pothos::serialization::throw_exception(
            xml_archive_exception(xml_archive_exception::xml_archive_parsing_error)
        );
    std::memcpy(s, tstring.data(), tstring.size());
    s[tstring.size()] = 0;
}

template<class Archive>
POTHOS_ARCHIVE_DECL(void)
xml_iarchive_impl<Archive>::load_override(class_name_type & t, int){
    const std::string & s = gimpl->rv.class_name;
    if(s.size() > POTHOS_SERIALIZATION_MAX_KEY_SIZE - 1)
        Pothos::serialization::throw_exception(
            archive_exception(archive_exception::invalid_class_name)
       );
    char * tptr = t;
    std::memcpy(tptr, s.data(), s.size());
    tptr[s.size()] = '\0';
}

template<class Archive>
POTHOS_ARCHIVE_DECL(void)
xml_iarchive_impl<Archive>::init(){
    gimpl->init(is);
    this->set_library_version(
        library_version_type(gimpl->rv.version)
    );
}

template<class Archive>
POTHOS_ARCHIVE_DECL(POTHOS_PP_EMPTY())
xml_iarchive_impl<Archive>::xml_iarchive_impl(
    std::istream &is_,
    unsigned int flags
) :
    basic_text_iprimitive<std::istream>(
        is_, 
        0 != (flags & no_codecvt)
    ),
    basic_xml_iarchive<Archive>(flags),
    gimpl(new xml_grammar())
{
    if(0 == (flags & no_header)){
        POTHOS_TRY{
            init();
        }
        POTHOS_CATCH(...){
            delete gimpl;
            #ifndef POTHOS_NO_EXCEPTIONS
                throw; // re-throw
            #endif
        }
        POTHOS_CATCH_END
    }
}

template<class Archive>
POTHOS_ARCHIVE_DECL(POTHOS_PP_EMPTY())
xml_iarchive_impl<Archive>::~xml_iarchive_impl(){
    if(0 == (this->get_flags() & no_header)){
        POTHOS_TRY{
            gimpl->windup(is);
        }
        POTHOS_CATCH(...){}
        POTHOS_CATCH_END
    }
    delete gimpl;
}
} // namespace archive
} // namespace boost
