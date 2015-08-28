/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// basic_text_iarchive.ipp:

// (C) Copyright 2002 Robert Ramey - http://www.rrsd.com . 
// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for updates, documentation, and revision history.
#include <string>
#include <algorithm>
#include <cstring>

#include <Pothos/serialization/impl/config.hpp> // for BOOST_DEDUCED_TYPENAME
#if defined(POTHOS_NO_STDC_NAMESPACE)
namespace std{ 
    using ::memcpy; 
}
#endif

#include <Pothos/serialization/impl/detail/workaround.hpp>
#include <Pothos/serialization/string.hpp>
#include <Pothos/archive/basic_text_iarchive.hpp>

namespace Pothos {
namespace archive {

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// implementation of text_text_archive

template<class Archive>
POTHOS_ARCHIVE_OR_WARCHIVE_DECL(void)
basic_text_iarchive<Archive>::load_override(class_name_type & t, int){
    std::string cn;
    cn.reserve(POTHOS_SERIALIZATION_MAX_KEY_SIZE);
    load_override(cn, 0);
    if(cn.size() > (POTHOS_SERIALIZATION_MAX_KEY_SIZE - 1))
        Pothos::serialization::throw_exception(
            archive_exception(archive_exception::invalid_class_name)
        );
    std::memcpy(t, cn.data(), cn.size());
    // borland tweak
    t.t[cn.size()] = '\0';
}

template<class Archive>
POTHOS_ARCHIVE_OR_WARCHIVE_DECL(void)
basic_text_iarchive<Archive>::init(void){
    // read signature in an archive version independent manner
    std::string file_signature;
    * this->This() >> file_signature;
    if(file_signature != POTHOS_ARCHIVE_SIGNATURE())
        Pothos::serialization::throw_exception(
            archive_exception(archive_exception::invalid_signature)
        );

    // make sure the version of the reading archive library can
    // support the format of the archive being read
    library_version_type input_library_version;
    * this->This() >> input_library_version;

    #if POTHOS_WORKAROUND(__MWERKS__, POTHOS_TESTED_AT(0x3205))
    this->set_library_version(input_library_version);
    #else
    #if ! POTHOS_WORKAROUND(POTHOS_MSVC, <= 1200)
    detail::
    #endif
    basic_iarchive::set_library_version(input_library_version);
    #endif

    // extra little .t is to get around borland quirk
    if(POTHOS_ARCHIVE_VERSION() < input_library_version)
        Pothos::serialization::throw_exception(
            archive_exception(archive_exception::unsupported_version)
        );
}

} // namespace archive
} // namespace boost
