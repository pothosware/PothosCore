#ifndef POTHOS_SERIALIZATION_TYPE_INFO_IMPLEMENTATION_HPP
#define POTHOS_SERIALIZATION_TYPE_INFO_IMPLEMENTATION_HPP

// MS compatible compilers support #pragma once
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// type_info_implementation.hpp: interface for portable version of type_info

// (C) Copyright 2002 Robert Ramey - http://www.rrsd.com . 
// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for updates, documentation, and revision history.


#include <Pothos/serialization/impl/config.hpp>
#include <Pothos/serialization/impl/detail/workaround.hpp>

#include <Pothos/serialization/impl/static_assert.hpp>
#include <Pothos/serialization/impl/mpl/eval_if.hpp>
#include <Pothos/serialization/impl/mpl/identity.hpp>
#include <Pothos/serialization/impl/type_traits/is_base_and_derived.hpp>
#include <Pothos/serialization/traits.hpp>

namespace Pothos {
namespace serialization {

// note that T and const T are folded into const T so that
// there is only one table entry per type
template<class T>
struct type_info_implementation {
    template<class U>
    struct traits_class_typeinfo_implementation {
      typedef POTHOS_DEDUCED_TYPENAME U::type_info_implementation::type type;
    };
    // note: at least one compiler complained w/o the full qualification
    // on basic traits below
    typedef 
        POTHOS_DEDUCED_TYPENAME mpl::eval_if<
            is_base_and_derived<Pothos::serialization::basic_traits, T>,
            traits_class_typeinfo_implementation< T >,
        //else
            mpl::identity<
                POTHOS_DEDUCED_TYPENAME extended_type_info_impl< T >::type
            >
        >::type type;
};

} // namespace serialization
} // namespace boost

// define a macro to assign a particular derivation of extended_type_info
// to a specified a class. 
#if POTHOS_WORKAROUND(__BORLANDC__, POTHOS_TESTED_AT(0x560))
#define POTHOS_CLASS_TYPE_INFO(T, ETI)              \
namespace Pothos {                                  \
namespace serialization {                          \
template<>                                         \
struct type_info_implementation< T > {             \
    typedef const ETI type;                        \
};                                                 \
}                                                  \
}                                                  \
/**/
#else
#define POTHOS_CLASS_TYPE_INFO(T, ETI)              \
namespace Pothos {                                  \
namespace serialization {                          \
template<>                                         \
struct type_info_implementation< T > {             \
    typedef ETI type;                              \
};                                                 \
template<>                                         \
struct type_info_implementation< const T > {       \
    typedef ETI type;                              \
};                                                 \
}                                                  \
}                                                  \
/**/
#endif

#endif /// BOOST_SERIALIZATION_TYPE_INFO_IMPLEMENTATION_HPP
