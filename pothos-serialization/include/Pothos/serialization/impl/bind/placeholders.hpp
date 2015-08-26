#ifndef POTHOS_BIND_PLACEHOLDERS_HPP_INCLUDED
#define POTHOS_BIND_PLACEHOLDERS_HPP_INCLUDED

// MS compatible compilers support #pragma once

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

//
//  bind/placeholders.hpp - _N definitions
//
//  Copyright (c) 2002 Peter Dimov and Multi Media Ltd.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
//  See http://www.boost.org/libs/bind/bind.html for documentation.
//

#include <Pothos/serialization/impl/bind/arg.hpp>
#include <Pothos/serialization/impl/config.hpp>

namespace
{

#if defined(__BORLANDC__) || defined(__GNUC__) && (__GNUC__ < 4)

static inline Pothos::arg<1> _1() { return Pothos::arg<1>(); }
static inline Pothos::arg<2> _2() { return Pothos::arg<2>(); }
static inline Pothos::arg<3> _3() { return Pothos::arg<3>(); }
static inline Pothos::arg<4> _4() { return Pothos::arg<4>(); }
static inline Pothos::arg<5> _5() { return Pothos::arg<5>(); }
static inline Pothos::arg<6> _6() { return Pothos::arg<6>(); }
static inline Pothos::arg<7> _7() { return Pothos::arg<7>(); }
static inline Pothos::arg<8> _8() { return Pothos::arg<8>(); }
static inline Pothos::arg<9> _9() { return Pothos::arg<9>(); }

#elif defined(POTHOS_MSVC) || (defined(__DECCXX_VER) && __DECCXX_VER <= 60590031) || defined(__MWERKS__) || \
    defined(__GNUC__) && (__GNUC__ == 4 && __GNUC_MINOR__ < 2)  

static Pothos::arg<1> _1;
static Pothos::arg<2> _2;
static Pothos::arg<3> _3;
static Pothos::arg<4> _4;
static Pothos::arg<5> _5;
static Pothos::arg<6> _6;
static Pothos::arg<7> _7;
static Pothos::arg<8> _8;
static Pothos::arg<9> _9;

#else

Pothos::arg<1> _1;
Pothos::arg<2> _2;
Pothos::arg<3> _3;
Pothos::arg<4> _4;
Pothos::arg<5> _5;
Pothos::arg<6> _6;
Pothos::arg<7> _7;
Pothos::arg<8> _8;
Pothos::arg<9> _9;

#endif

} // unnamed namespace

#endif // #ifndef BOOST_BIND_PLACEHOLDERS_HPP_INCLUDED
