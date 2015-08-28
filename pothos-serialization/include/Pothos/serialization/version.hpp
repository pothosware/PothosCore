#ifndef POTHOS_SERIALIZATION_VERSION_HPP
#define POTHOS_SERIALIZATION_VERSION_HPP

// MS compatible compilers support #pragma once
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// version.hpp:

// (C) Copyright 2002 Robert Ramey - http://www.rrsd.com . 
// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for updates, documentation, and revision history.

#include <Pothos/serialization/impl/config.hpp>
#include <Pothos/serialization/impl/mpl/assert.hpp>
#include <Pothos/serialization/impl/mpl/int.hpp>
#include <Pothos/serialization/impl/mpl/eval_if.hpp>
#include <Pothos/serialization/impl/mpl/identity.hpp>
#include <Pothos/serialization/impl/mpl/integral_c_tag.hpp>

#include <Pothos/serialization/impl/type_traits/is_base_and_derived.hpp>

namespace Pothos { 
namespace serialization {

struct basic_traits;

// default version number is 0. Override with higher version
// when class definition changes.
template<class T>
struct version
{
    template<class U>
    struct traits_class_version {
        typedef POTHOS_DEDUCED_TYPENAME U::version type;
    };

    typedef mpl::integral_c_tag tag;
    // note: at least one compiler complained w/o the full qualification
    // on basic traits below
    typedef
        POTHOS_DEDUCED_TYPENAME mpl::eval_if<
            is_base_and_derived<Pothos::serialization::basic_traits,T>,
            traits_class_version< T >,
            mpl::int_<0>
        >::type type;
    POTHOS_STATIC_CONSTANT(int, value = version::type::value);
};

#ifndef POTHOS_NO_INCLASS_MEMBER_INITIALIZATION
template<class T>
const int version<T>::value;
#endif

} // namespace serialization
} // namespace boost

/* note: at first it seemed that this would be a good place to trap
 * as an error an attempt to set a version # for a class which doesn't
 * save its class information (including version #) in the archive.
 * However, this imposes a requirement that the version be set after
 * the implemention level which would be pretty confusing.  If this
 * is to be done, do this check in the input or output operators when
 * ALL the serialization traits are available.  Included the implementation
 * here with this comment as a reminder not to do this!
 */
//#include <boost/serialization/level.hpp>
//#include <boost/mpl/equal_to.hpp>

#include <Pothos/serialization/impl/mpl/less.hpp>
#include <Pothos/serialization/impl/mpl/comparison.hpp>

// specify the current version number for the class
// version numbers limited to 8 bits !!!
#define POTHOS_CLASS_VERSION(T, N)                                      \
namespace Pothos {                                                      \
namespace serialization {                                              \
template<>                                                             \
struct version<T >                                                     \
{                                                                      \
    typedef mpl::int_<N> type;                                         \
    typedef mpl::integral_c_tag tag;                                   \
    POTHOS_STATIC_CONSTANT(int, value = version::type::value);          \
    POTHOS_MPL_ASSERT((                                                 \
        Pothos::mpl::less<                                              \
            Pothos::mpl::int_<N>,                                       \
            Pothos::mpl::int_<256>                                      \
        >                                                              \
    ));                                                                \
    /*                                                                 \
    POTHOS_MPL_ASSERT((                                                 \
        mpl::equal_to<                                                 \
            :implementation_level<T >,                                 \
            mpl::int_<object_class_info>                               \
        >::value                                                       \
    ));                                                                \
    */                                                                 \
};                                                                     \
}                                                                      \
}

#endif // BOOST_SERIALIZATION_VERSION_HPP
