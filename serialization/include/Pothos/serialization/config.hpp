#ifndef POTHOS_SERIALIZATION_CONFIG_HPP
#define POTHOS_SERIALIZATION_CONFIG_HPP

//  config.hpp  ---------------------------------------------//

//  (c) Copyright Robert Ramey 2004
//  Use, modification, and distribution is subject to the Boost Software
//  License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

//  See library home page at http://www.boost.org/libs/serialization

//----------------------------------------------------------------------------// 

// This header implements separate compilation features as described in
// http://www.boost.org/more/separate_compilation.html

#include <Pothos/serialization/impl/config.hpp>
#include <Pothos/serialization/impl/detail/workaround.hpp>
#include <Pothos/serialization/impl/preprocessor/facilities/empty.hpp>

// note: this version incorporates the related code into the the 
// the same library as BOOST_ARCHIVE.  This could change some day in the
// future

// if BOOST_SERIALIZATION_DECL is defined undefine it now:
#ifdef POTHOS_SERIALIZATION_DECL
    #undef POTHOS_SERIALIZATION_DECL
#endif

#ifdef POTHOS_HAS_DECLSPEC // defined in config system
// we need to import/export our code only if the user has specifically
// asked for it by defining either BOOST_ALL_DYN_LINK if they want all boost
// libraries to be dynamically linked, or BOOST_SERIALIZATION_DYN_LINK
// if they want just this one to be dynamically liked:
#if defined(POTHOS_ALL_DYN_LINK) || defined(POTHOS_SERIALIZATION_DYN_LINK)
    #if !defined(POTHOS_DYN_LINK)
        #define POTHOS_DYN_LINK
    #endif
    // export if this is our own source, otherwise import:
    #if defined(POTHOS_SERIALIZATION_SOURCE)
        #if defined(__BORLANDC__)
            #define POTHOS_SERIALIZATION_DECL(T) T __export
        #else
            #define POTHOS_SERIALIZATION_DECL(T) __declspec(dllexport) T
        #endif
    #else
        #if defined(__BORLANDC__)
            #define POTHOS_SERIALIZATION_DECL(T) T __import
        #else
            #define POTHOS_SERIALIZATION_DECL(T) __declspec(dllimport) T
        #endif
    #endif // defined(BOOST_SERIALIZATION_SOURCE)
#endif // defined(BOOST_ALL_DYN_LINK) || defined(BOOST_SERIALIZATION_DYN_LINK)
#endif // BOOST_HAS_DECLSPEC

// if BOOST_SERIALIZATION_DECL isn't defined yet define it now:
#ifndef POTHOS_SERIALIZATION_DECL
    #define POTHOS_SERIALIZATION_DECL(T) T
#endif

//  enable automatic library variant selection  ------------------------------// 

#if !defined(POTHOS_ALL_NO_LIB) && !defined(POTHOS_SERIALIZATION_NO_LIB) \
&&  !defined(POTHOS_ARCHIVE_SOURCE) && !defined(POTHOS_WARCHIVE_SOURCE)  \
&&  !defined(POTHOS_SERIALIZATION_SOURCE)
    //
    // Set the name of our library, this will get undef'ed by auto_link.hpp
    // once it's done with it:
    //
    #define POTHOS_LIB_NAME Pothos_serialization
    //
    // If we're importing code from a dll, then tell auto_link.hpp about it:
    //
    #if defined(POTHOS_ALL_DYN_LINK) || defined(POTHOS_SERIALIZATION_DYN_LINK)
    #  define POTHOS_DYN_LINK
    #endif
    //
    // And include the header that does the work:
    //
    #include <Pothos/serialization/impl/config/auto_link.hpp>

#endif  

#endif // BOOST_SERIALIZATION_CONFIG_HPP
