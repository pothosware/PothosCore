#ifndef POTHOS_ARCHIVE_POLYMORPHIC_XML_OARCHIVE_HPP
#define POTHOS_ARCHIVE_POLYMORPHIC_XML_OARCHIVE_HPP

// MS compatible compilers support #pragma once
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// polymorphic_xml_oarchive.hpp

// (C) Copyright 2002 Robert Ramey - http://www.rrsd.com . 
// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for updates, documentation, and revision history.

#include <Pothos/serialization/impl/config.hpp>
#include <Pothos/archive/xml_oarchive.hpp>
#include <Pothos/archive/detail/polymorphic_oarchive_route.hpp>

namespace Pothos { 
namespace archive {

typedef detail::polymorphic_oarchive_route<
    xml_oarchive_impl<naked_xml_oarchive> 
> polymorphic_xml_oarchive;

} // namespace archive
} // namespace boost

// required by export
POTHOS_SERIALIZATION_REGISTER_ARCHIVE(
    Pothos::archive::polymorphic_xml_oarchive
)

#endif // BOOST_ARCHIVE_POLYMORPHIC_XML_OARCHIVE_HPP

