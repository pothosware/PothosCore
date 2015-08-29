//    Copyright (c) 2004 Hartmut Kaiser
//
//    Use, modification and distribution is subject to the Boost Software
//    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//    http://www.boost.org/LICENSE_1_0.txt)

#ifndef POTHOS_PROGRAM_OPTIONS_CONFIG_HK_2004_01_11
#define POTHOS_PROGRAM_OPTIONS_CONFIG_HK_2004_01_11

#include <Pothos/serialization/impl/config.hpp>
#include <Pothos/serialization/impl/version.hpp>

// Support for autolinking.
#if POTHOS_VERSION >= 103100   // works beginning from Boost V1.31.0

///////////////////////////////////////////////////////////////////////////////
// enable automatic library variant selection 
#if !defined(POTHOS_PROGRAM_OPTIONS_SOURCE) && !defined(POTHOS_ALL_NO_LIB) && \
    !defined(POTHOS_PROGRAM_OPTIONS_NO_LIB)

// Set the name of our library, this will get undef'ed by auto_link.hpp
// once it's done with it:
#define POTHOS_LIB_NAME Pothos_program_options
// tell the auto-link code to select a dll when required:
#if defined(POTHOS_ALL_DYN_LINK) || defined(POTHOS_PROGRAM_OPTIONS_DYN_LINK)
#  define POTHOS_DYN_LINK
#endif

// And include the header that does the work:
#include <Pothos/serialization/impl/config/auto_link.hpp>

#endif  // auto-linking disabled

#endif  // BOOST_VERSION

///////////////////////////////////////////////////////////////////////////////
#if defined(POTHOS_ALL_DYN_LINK) || defined(POTHOS_PROGRAM_OPTIONS_DYN_LINK)
// export if this is our own source, otherwise import:
#ifdef POTHOS_PROGRAM_OPTIONS_SOURCE
# define POTHOS_PROGRAM_OPTIONS_DECL POTHOS_SYMBOL_EXPORT
#else
# define POTHOS_PROGRAM_OPTIONS_DECL POTHOS_SYMBOL_IMPORT
#endif  // BOOST_PROGRAM_OPTIONS_SOURCE
#endif  // DYN_LINK

#ifndef POTHOS_PROGRAM_OPTIONS_DECL
#define POTHOS_PROGRAM_OPTIONS_DECL
#endif


#endif // PROGRAM_OPTIONS_CONFIG_HK_2004_01_11

