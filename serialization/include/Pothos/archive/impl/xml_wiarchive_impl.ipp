/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// xml_wiprimitive.cpp:

// (C) Copyright 2002 Robert Ramey - http://www.rrsd.com .
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for updates, documentation, and revision history.

#include <Pothos/serialization/impl/config.hpp> // for BOOST_DEDUCED_TYPENAME

#include <cstring>
#if defined(POTHOS_NO_STDC_NAMESPACE)
namespace std{ 
    using ::memcpy; 
} //std
#endif

#include <Pothos/serialization/impl/config.hpp> // msvc 6.0 needs this to suppress warnings
#ifndef POTHOS_NO_STD_WSTREAMBUF

#include <Pothos/serialization/impl/assert.hpp>
#include <algorithm>

#include <Pothos/serialization/impl/detail/workaround.hpp> // Dinkumware and RogueWave
#if POTHOS_WORKAROUND(POTHOS_DINKUMWARE_STDLIB, == 1)
#include <Pothos/archive/dinkumware.hpp>
#endif

#include <Pothos/serialization/impl/io/ios_state.hpp>
#include <Pothos/serialization/impl/detail/no_exceptions_support.hpp>
#include <Pothos/serialization/pfto.hpp>

#include <Pothos/serialization/string.hpp>
#include <Pothos/archive/add_facet.hpp>
#include <Pothos/archive/xml_archive_exception.hpp>
#include <Pothos/archive/detail/utf8_codecvt_facet.hpp>

#include <Pothos/archive/iterators/mb_from_wchar.hpp>

#include <Pothos/archive/basic_xml_archive.hpp>
#include <Pothos/archive/xml_wiarchive.hpp>

#include "basic_xml_grammar.hpp"

namespace Pothos {
namespace archive {

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// implemenations of functions specific to wide char archives

namespace { // anonymous

void copy_to_ptr(char * s, const std::wstring & ws){
    std::copy(
        iterators::mb_from_wchar<std::wstring::const_iterator>(
            POTHOS_MAKE_PFTO_WRAPPER(ws.begin())
        ), 
        iterators::mb_from_wchar<std::wstring::const_iterator>(
            POTHOS_MAKE_PFTO_WRAPPER(ws.end())
        ), 
        s
    );
    s[ws.size()] = 0;
}

} // anonymous

template<class Archive>
POTHOS_WARCHIVE_DECL(void)
xml_wiarchive_impl<Archive>::load(std::string & s){
    std::wstring ws;
    bool result = gimpl->parse_string(is, ws);
    if(! result)
        Pothos::serialization::throw_exception(
            xml_archive_exception(xml_archive_exception::xml_archive_parsing_error)
        );
    #if POTHOS_WORKAROUND(_RWSTD_VER, POTHOS_TESTED_AT(20101))
    if(NULL != s.data())
    #endif
        s.resize(0);
    s.reserve(ws.size());
    std::copy(
        iterators::mb_from_wchar<std::wstring::iterator>(
            POTHOS_MAKE_PFTO_WRAPPER(ws.begin())
        ), 
        iterators::mb_from_wchar<std::wstring::iterator>(
            POTHOS_MAKE_PFTO_WRAPPER(ws.end())
        ), 
        std::back_inserter(s)
    );
}

#ifndef POTHOS_NO_STD_WSTRING
template<class Archive>
POTHOS_WARCHIVE_DECL(void)
xml_wiarchive_impl<Archive>::load(std::wstring & ws){
    bool result = gimpl->parse_string(is, ws);
    if(! result)
        Pothos::serialization::throw_exception(
            xml_archive_exception(xml_archive_exception::xml_archive_parsing_error)
        );
}
#endif

template<class Archive>
POTHOS_WARCHIVE_DECL(void)
xml_wiarchive_impl<Archive>::load(char * s){
    std::wstring ws;
    bool result = gimpl->parse_string(is, ws);
    if(! result)
        Pothos::serialization::throw_exception(
            xml_archive_exception(xml_archive_exception::xml_archive_parsing_error)
        );
    copy_to_ptr(s, ws);
}

#ifndef POTHOS_NO_INTRINSIC_WCHAR_T
template<class Archive>
POTHOS_WARCHIVE_DECL(void)
xml_wiarchive_impl<Archive>::load(wchar_t * ws){
    std::wstring twstring;
    bool result = gimpl->parse_string(is, twstring);
    if(! result)
        Pothos::serialization::throw_exception(
            xml_archive_exception(xml_archive_exception::xml_archive_parsing_error)
        );
    std::memcpy(ws, twstring.c_str(), twstring.size());
    ws[twstring.size()] = L'\0';
}
#endif

template<class Archive>
POTHOS_WARCHIVE_DECL(void)
xml_wiarchive_impl<Archive>::load_override(class_name_type & t, int){
    const std::wstring & ws = gimpl->rv.class_name;
    if(ws.size() > POTHOS_SERIALIZATION_MAX_KEY_SIZE - 1)
        Pothos::serialization::throw_exception(
            archive_exception(archive_exception::invalid_class_name)
        );
    copy_to_ptr(t, ws);
}

template<class Archive>
POTHOS_WARCHIVE_DECL(void)
xml_wiarchive_impl<Archive>::init(){
    gimpl->init(is);
    this->set_library_version(
        library_version_type(gimpl->rv.version)
    );
}

template<class Archive>
POTHOS_WARCHIVE_DECL(POTHOS_PP_EMPTY())
xml_wiarchive_impl<Archive>::xml_wiarchive_impl(
    std::wistream &is_,
    unsigned int flags
) :
    basic_text_iprimitive<std::wistream>(
        is_, 
        true // don't change the codecvt - use the one below
    ),
    basic_xml_iarchive<Archive>(flags),
    gimpl(new xml_wgrammar())
{
    if(0 == (flags & no_codecvt)){
        archive_locale.reset(
            add_facet(
                std::locale::classic(),
                new Pothos::archive::detail::utf8_codecvt_facet
            )
        );
        is.imbue(* archive_locale);
    }
    if(0 == (flags & no_header)){
        POTHOS_TRY{
            this->init();
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
POTHOS_WARCHIVE_DECL(POTHOS_PP_EMPTY())
xml_wiarchive_impl<Archive>::~xml_wiarchive_impl(){
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

#endif  // BOOST_NO_STD_WSTREAMBUF
