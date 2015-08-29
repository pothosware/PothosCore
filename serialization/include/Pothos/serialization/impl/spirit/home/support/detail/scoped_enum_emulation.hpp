//  Copyright (c) 2001-2011 Hartmut Kaiser
//  http://spirit.sourceforge.net/
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef POTHOS_SPIRIT_SCOPED_ENUM_EMULATION_HPP
#define POTHOS_SPIRIT_SCOPED_ENUM_EMULATION_HPP

#if defined(_MSC_VER)
#pragma once
#endif

#include <Pothos/serialization/impl/version.hpp>
#include <Pothos/serialization/impl/config.hpp>

#if POTHOS_VERSION >= 104000
# include <Pothos/serialization/impl/detail/scoped_enum_emulation.hpp>
#else
# if !defined(POTHOS_NO_CXX11_SCOPED_ENUMS)
#  define POTHOS_NO_CXX11_SCOPED_ENUMS
# endif 
# define POTHOS_SCOPED_ENUM_START(name) struct name { enum enum_type
# define POTHOS_SCOPED_ENUM_END };
# define POTHOS_SCOPED_ENUM(name) name::enum_type
#endif

#endif
