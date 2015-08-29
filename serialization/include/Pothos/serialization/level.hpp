#ifndef POTHOS_SERIALIZATION_LEVEL_HPP
#define POTHOS_SERIALIZATION_LEVEL_HPP

// MS compatible compilers support #pragma once
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// level.hpp:

// (C) Copyright 2002 Robert Ramey - http://www.rrsd.com . 
// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for updates, documentation, and revision history.

#include <Pothos/serialization/impl/config.hpp>
#include <Pothos/serialization/impl/detail/workaround.hpp>

#include <Pothos/serialization/impl/type_traits/is_fundamental.hpp>
#include <Pothos/serialization/impl/type_traits/is_enum.hpp>
#include <Pothos/serialization/impl/type_traits/is_array.hpp>
#include <Pothos/serialization/impl/type_traits/is_class.hpp>
#include <Pothos/serialization/impl/type_traits/is_base_and_derived.hpp>

#include <Pothos/serialization/impl/mpl/eval_if.hpp>
#include <Pothos/serialization/impl/mpl/int.hpp>
#include <Pothos/serialization/impl/mpl/integral_c.hpp>
#include <Pothos/serialization/impl/mpl/integral_c_tag.hpp>
#include <Pothos/serialization/impl/mpl/aux_/nttp_decl.hpp>

#include <Pothos/serialization/level_enum.hpp>

namespace Pothos {
namespace serialization {

struct basic_traits;

// default serialization implementation level
template<class T>
struct implementation_level_impl {
    template<class U>
    struct traits_class_level {
        typedef POTHOS_DEDUCED_TYPENAME U::level type;
    };

    typedef mpl::integral_c_tag tag;
    // note: at least one compiler complained w/o the full qualification
    // on basic traits below
    typedef
        POTHOS_DEDUCED_TYPENAME mpl::eval_if<
            is_base_and_derived<Pothos::serialization::basic_traits, T>,
            traits_class_level< T >,
        //else
        POTHOS_DEDUCED_TYPENAME mpl::eval_if<
            is_fundamental< T >,
            mpl::int_<primitive_type>,
        //else
        POTHOS_DEDUCED_TYPENAME mpl::eval_if<
            is_class< T >,
            mpl::int_<object_class_info>,
        //else
        POTHOS_DEDUCED_TYPENAME mpl::eval_if<
            is_array< T >,
            #if POTHOS_WORKAROUND(__BORLANDC__, POTHOS_TESTED_AT(0x560))
                mpl::int_<not_serializable>,
            #else
                mpl::int_<object_serializable>,
            #endif
        //else
        POTHOS_DEDUCED_TYPENAME mpl::eval_if<
            is_enum< T >,
            //#if BOOST_WORKAROUND(__BORLANDC__, BOOST_TESTED_AT(0x560))
            //    mpl::int_<not_serializable>,
            //#else
                mpl::int_<primitive_type>,
            //#endif
        //else
            mpl::int_<not_serializable>
        >
        >
        >
        >
        >::type type;
        // vc 7.1 doesn't like enums here
    POTHOS_STATIC_CONSTANT(int, value = type::value);
};

template<class T>
struct implementation_level : 
    public implementation_level_impl<const T>
{
};

template<class T, POTHOS_MPL_AUX_NTTP_DECL(int, L) >
inline bool operator>=(implementation_level< T > t, enum level_type l)
{
    return t.value >= (int)l;
}

} // namespace serialization
} // namespace boost

// specify the level of serialization implementation for the class
// require that class info saved when versioning is used
#define POTHOS_CLASS_IMPLEMENTATION(T, E)                 \
    namespace Pothos {                                    \
    namespace serialization {                            \
    template <>                                          \
    struct implementation_level_impl< const T >                     \
    {                                                    \
        typedef mpl::integral_c_tag tag;                 \
        typedef mpl::int_< E > type;                     \
        POTHOS_STATIC_CONSTANT(                           \
            int,                                         \
            value = implementation_level_impl::type::value    \
        );                                               \
    };                                                   \
    }                                                    \
    }
    /**/

#endif // BOOST_SERIALIZATION_LEVEL_HPP
