#ifndef POTHOS_SERIALIZATION_SPLIT_MEMBER_HPP
#define POTHOS_SERIALIZATION_SPLIT_MEMBER_HPP

// MS compatible compilers support #pragma once
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// split_member.hpp:

// (C) Copyright 2002 Robert Ramey - http://www.rrsd.com . 
// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for updates, documentation, and revision history.

#include <Pothos/serialization/impl/config.hpp>
#include <Pothos/serialization/impl/mpl/eval_if.hpp>
#include <Pothos/serialization/impl/mpl/identity.hpp>

#include <Pothos/serialization/access.hpp>

namespace Pothos {
namespace archive {
    namespace detail {
        template<class Archive> class interface_oarchive;
        template<class Archive> class interface_iarchive;
    } // namespace detail
} // namespace archive

namespace serialization {
namespace detail {

    template<class Archive, class T>
    struct member_saver {
        static void invoke(
            Archive & ar, 
            const T & t,
            const unsigned int file_version
        ){
            access::member_save(ar, t, file_version);
        }
    };

    template<class Archive, class T>
    struct member_loader {
        static void invoke(
            Archive & ar, 
            T & t,
            const unsigned int file_version
        ){
            access::member_load(ar, t, file_version);
        }
    };

} // detail

template<class Archive, class T>
inline void split_member(
    Archive & ar, T & t, const unsigned int file_version
){
    typedef POTHOS_DEDUCED_TYPENAME mpl::eval_if<
        POTHOS_DEDUCED_TYPENAME Archive::is_saving,
        mpl::identity<detail::member_saver<Archive, T> >, 
        mpl::identity<detail::member_loader<Archive, T> >
    >::type typex;
    typex::invoke(ar, t, file_version);
}

} // namespace serialization
} // namespace boost

// split member function serialize funcition into save/load
#define POTHOS_SERIALIZATION_SPLIT_MEMBER()                       \
template<class Archive>                                          \
void serialize(                                                  \
    Archive &ar,                                                 \
    const unsigned int file_version                              \
){                                                               \
    Pothos::serialization::split_member(ar, *this, file_version); \
}                                                                \
/**/

#endif // BOOST_SERIALIZATION_SPLIT_MEMBER_HPP
