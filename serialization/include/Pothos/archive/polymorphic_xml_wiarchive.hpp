#ifndef POTHOS_ARCHIVE_POLYMORPHIC_XML_WIARCHIVE_HPP
#define POTHOS_ARCHIVE_POLYMORPHIC_XML_WIARCHIVE_HPP

// MS compatible compilers support #pragma once
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// polymorphic_xml_wiarchive.hpp

// (C) Copyright 2002 Robert Ramey - http://www.rrsd.com . 
// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for updates, documentation, and revision history.

#include <Pothos/serialization/impl/config.hpp>
#ifdef POTHOS_NO_STD_WSTREAMBUF
#error "wide char i/o not supported on this platform"
#else

#include <Pothos/archive/xml_wiarchive.hpp>
#include <Pothos/archive/detail/polymorphic_iarchive_route.hpp>

namespace Pothos { 
namespace archive {

class polymorphic_xml_wiarchive : 
    public detail::polymorphic_iarchive_route<naked_xml_wiarchive>
{
public:
    polymorphic_xml_wiarchive(std::wistream & is, unsigned int flags = 0) :
        detail::polymorphic_iarchive_route<naked_xml_wiarchive>(is, flags)
    {}
    ~polymorphic_xml_wiarchive(){}
};

} // namespace archive
} // namespace boost

// required by export
POTHOS_SERIALIZATION_REGISTER_ARCHIVE(
    Pothos::archive::polymorphic_xml_wiarchive
)

#endif // BOOST_NO_STD_WSTREAMBUF 
#endif // BOOST_ARCHIVE_POLYMORPHIC_XML_WIARCHIVE_HPP

