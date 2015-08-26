#ifndef POTHOS_SERIALIZATION_SHARED_PTR_HPP
#define POTHOS_SERIALIZATION_SHARED_PTR_HPP

// MS compatible compilers support #pragma once
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// shared_ptr.hpp: serialization for boost shared pointer

// (C) Copyright 2004 Robert Ramey and Martin Ecker
// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for updates, documentation, and revision history.

#include <cstddef> // NULL

#include <Pothos/serialization/impl/config.hpp>
#include <Pothos/serialization/impl/mpl/integral_c.hpp>
#include <Pothos/serialization/impl/mpl/integral_c_tag.hpp>

#include <Pothos/serialization/impl/detail/workaround.hpp>
#include <Pothos/serialization/impl/shared_ptr.hpp>

#include <Pothos/serialization/split_free.hpp>
#include <Pothos/serialization/nvp.hpp>
#include <Pothos/serialization/version.hpp>
#include <Pothos/serialization/tracking.hpp>

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// shared_ptr serialization traits
// version 1 to distinguish from boost 1.32 version. Note: we can only do this
// for a template when the compiler supports partial template specialization

#ifndef POTHOS_NO_TEMPLATE_PARTIAL_SPECIALIZATION
    namespace Pothos {
    namespace serialization{
        template<class T>
        struct version< ::Pothos::shared_ptr< T > > {
            typedef mpl::integral_c_tag tag;
            #if POTHOS_WORKAROUND(__MWERKS__, POTHOS_TESTED_AT(0x3206))
            typedef POTHOS_DEDUCED_TYPENAME mpl::int_<1> type;
            #else
            typedef mpl::int_<1> type;
            #endif
            #if POTHOS_WORKAROUND(__BORLANDC__, POTHOS_TESTED_AT(0x570))
            POTHOS_STATIC_CONSTANT(int, value = 1);
            #else
            POTHOS_STATIC_CONSTANT(int, value = type::value);
            #endif
        };
        // don't track shared pointers
        template<class T>
        struct tracking_level< ::Pothos::shared_ptr< T > > { 
            typedef mpl::integral_c_tag tag;
            #if POTHOS_WORKAROUND(__MWERKS__, POTHOS_TESTED_AT(0x3206))
            typedef POTHOS_DEDUCED_TYPENAME mpl::int_< ::Pothos::serialization::track_never> type;
            #else
            typedef mpl::int_< ::Pothos::serialization::track_never> type;
            #endif
            #if POTHOS_WORKAROUND(__BORLANDC__, POTHOS_TESTED_AT(0x570))
            POTHOS_STATIC_CONSTANT(int, value = ::Pothos::serialization::track_never);
            #else
            POTHOS_STATIC_CONSTANT(int, value = type::value);
            #endif
        };
    }}
    #define POTHOS_SERIALIZATION_SHARED_PTR(T)
#else
    // define macro to let users of these compilers do this
    #define POTHOS_SERIALIZATION_SHARED_PTR(T)                         \
    POTHOS_CLASS_VERSION(                                              \
        ::Pothos::shared_ptr< T >,                                     \
        1                                                             \
    )                                                                 \
    POTHOS_CLASS_TRACKING(                                             \
        ::Pothos::shared_ptr< T >,                                     \
        ::Pothos::serialization::track_never                           \
    )                                                                 \
    /**/
#endif

namespace Pothos {
namespace serialization{

struct null_deleter {
    void operator()(void const *) const {}
};

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// serialization for shared_ptr

template<class Archive, class T>
inline void save(
    Archive & ar,
    const Pothos::shared_ptr< T > &t,
    const unsigned int /* file_version */
){
    // The most common cause of trapping here would be serializing
    // something like shared_ptr<int>.  This occurs because int
    // is never tracked by default.  Wrap int in a trackable type
    POTHOS_STATIC_ASSERT((tracking_level< T >::value != track_never));
    const T * t_ptr = t.get();
    ar << Pothos::serialization::make_nvp("px", t_ptr);
}

#ifdef POTHOS_SERIALIZATION_SHARED_PTR_132_HPP
template<class Archive, class T>
inline void load(
    Archive & ar,
    Pothos::shared_ptr< T > &t,
    const unsigned int file_version
){
    // The most common cause of trapping here would be serializing
    // something like shared_ptr<int>.  This occurs because int
    // is never tracked by default.  Wrap int in a trackable type
    POTHOS_STATIC_ASSERT((tracking_level< T >::value != track_never));
    T* r;
    if(file_version < 1){
        //ar.register_type(static_cast<
        //    boost_132::detail::sp_counted_base_impl<T *, boost::checked_deleter< T > > *
        //>(NULL));
        ar.register_type(static_cast<
            Pothos_132::detail::sp_counted_base_impl<T *, null_deleter > *
        >(NULL));
        Pothos_132::shared_ptr< T > sp;
        ar >> Pothos::serialization::make_nvp("px", sp.px);
        ar >> Pothos::serialization::make_nvp("pn", sp.pn);
        // got to keep the sps around so the sp.pns don't disappear
        ar.append(sp);
        r = sp.get();
    }
    else{
        ar >> Pothos::serialization::make_nvp("px", r);
    }
    ar.reset(t,r);
}

#else
template<class Archive, class T>
inline void load(
    Archive & ar,
    Pothos::shared_ptr< T > &t,
    const unsigned int /*file_version*/
){
    // The most common cause of trapping here would be serializing
    // something like shared_ptr<int>.  This occurs because int
    // is never tracked by default.  Wrap int in a trackable type
    POTHOS_STATIC_ASSERT((tracking_level< T >::value != track_never));
    T* r;
    ar >> Pothos::serialization::make_nvp("px", r);
    ar.reset(t,r);
}
#endif

template<class Archive, class T>
inline void serialize(
    Archive & ar,
    Pothos::shared_ptr< T > &t,
    const unsigned int file_version
){
    // correct shared_ptr serialization depends upon object tracking
    // being used.
    POTHOS_STATIC_ASSERT(
        Pothos::serialization::tracking_level< T >::value
        != Pothos::serialization::track_never
    );
    Pothos::serialization::split_free(ar, t, file_version);
}

} // namespace serialization
} // namespace boost

#endif // BOOST_SERIALIZATION_SHARED_PTR_HPP
