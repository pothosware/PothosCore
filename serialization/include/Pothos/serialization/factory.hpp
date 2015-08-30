#ifndef POTHOS_SERIALIZATION_FACTORY_HPP
#define POTHOS_SERIALIZATION_FACTORY_HPP

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// MS compatible compilers support #pragma once
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

// factory.hpp: create an instance from an extended_type_info instance.

// (C) Copyright 2002 Robert Ramey - http://www.rrsd.com . 
// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for updates, documentation, and revision history.

#include <cstdarg> // valist
#include <cstddef> // NULL

#include <Pothos/serialization/impl/preprocessor/control/if.hpp> 
#include <Pothos/serialization/impl/preprocessor/comparison/greater.hpp>
#include <Pothos/serialization/impl/preprocessor/facilities/empty.hpp>

namespace std{
    #if defined(__LIBCOMO__)
        using ::va_list;
    #endif
} // namespace std

namespace Pothos {
namespace serialization {

// default implementation does nothing.
template<class T, int N>
T * factory(std::va_list){
    POTHOS_ASSERT(false);
    // throw exception here?
    return NULL;
}

} // namespace serialization
} // namespace boost

#define POTHOS_SERIALIZATION_FACTORY(N, T, A0, A1, A2, A3) \
namespace Pothos {                                         \
namespace serialization {                                 \
    template<>                                            \
    T * factory<T, N>(std::va_list ap){                   \
        POTHOS_PP_IF(POTHOS_PP_GREATER(N,0)                 \
            ,A0 a0 = va_arg(ap, A0);                      \
        ,POTHOS_PP_IF(POTHOS_PP_GREATER(N,1)                \
            ,A1 a1 = va_arg(ap, A1);                      \
        ,POTHOS_PP_IF(POTHOS_PP_GREATER(N,2)                \
            ,A2 a2 = va_arg(ap, A2);                      \
        ,POTHOS_PP_IF(POTHOS_PP_GREATER(N,3)                \
            ,A3 a3 = va_arg(ap, A3);                      \
            ,POTHOS_PP_EMPTY()                             \
        ))))                                              \
        return new T(                                     \
            POTHOS_PP_IF(POTHOS_PP_GREATER(N,0)             \
                ,a0                                       \
            ,POTHOS_PP_IF(POTHOS_PP_GREATER(N,1)            \
                ,a1                                       \
            ,POTHOS_PP_IF(POTHOS_PP_GREATER(N,2)            \
                ,a2                                       \
            ,POTHOS_PP_IF(POTHOS_PP_GREATER(N,3)            \
                ,a3                                       \
                ,POTHOS_PP_EMPTY()                         \
            ))))                                          \
        );                                                \
    }                                                     \
}                                                         \
}                                                         \
/**/

#define POTHOS_SERIALIZATION_FACTORY_4(T, A0, A1, A2, A3) \
    POTHOS_SERIALIZATION_FACTORY(4, T, A0, A1, A2, A3)

#define POTHOS_SERIALIZATION_FACTORY_3(T, A0, A1, A2)     \
    POTHOS_SERIALIZATION_FACTORY(3, T, A0, A1, A2, 0)

#define POTHOS_SERIALIZATION_FACTORY_2(T, A0, A1)         \
    POTHOS_SERIALIZATION_FACTORY(2, T, A0, A1, 0, 0)

#define POTHOS_SERIALIZATION_FACTORY_1(T, A0)             \
    POTHOS_SERIALIZATION_FACTORY(1, T, A0, 0, 0, 0)

#define POTHOS_SERIALIZATION_FACTORY_0(T)                 \
namespace Pothos {                                        \
namespace serialization {                                \
    template<>                                           \
    T * factory<T, 0>(std::va_list){                     \
        return new T();                                  \
    }                                                    \
}                                                        \
}                                                        \
/**/

#endif // BOOST_SERIALIZATION_FACTORY_HPP
