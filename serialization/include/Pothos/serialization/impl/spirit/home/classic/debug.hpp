/*=============================================================================
    Copyright (c) 2001-2003 Joel de Guzman
    Copyright (c) 2002-2003 Hartmut Kaiser
    http://spirit.sourceforge.net/

  Distributed under the Boost Software License, Version 1.0. (See accompanying
  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#if !defined(POTHOS_SPIRIT_DEBUG_MAIN_HPP)
#define POTHOS_SPIRIT_DEBUG_MAIN_HPP

///////////////////////////////////////////////////////////////////////////
#if defined(POTHOS_SPIRIT_DEBUG)

#include <Pothos/serialization/impl/spirit/home/classic/version.hpp>

///////////////////////////////////////////////////////////////////////////////
//
//  Spirit.Debug includes and defines
//
///////////////////////////////////////////////////////////////////////////////

    #include <iostream>

    ///////////////////////////////////////////////////////////////////////////
    //
    //  The BOOST_SPIRIT_DEBUG_OUT defines the stream object, which should be used
    //  for debug diagnostics. This defaults to std::cout.
    //
    ///////////////////////////////////////////////////////////////////////////
    #if !defined(POTHOS_SPIRIT_DEBUG_OUT)
    #define POTHOS_SPIRIT_DEBUG_OUT std::cout
    #endif

    ///////////////////////////////////////////////////////////////////////////
    //
    //  The BOOST_SPIRIT_DEBUG_PRINT_SOME constant defines the number of characters
    //  from the stream to be printed for diagnosis. This defaults to the first
    //  20 characters.
    //
    ///////////////////////////////////////////////////////////////////////////
    #if !defined(POTHOS_SPIRIT_DEBUG_PRINT_SOME)
    #define POTHOS_SPIRIT_DEBUG_PRINT_SOME 20
    #endif

    ///////////////////////////////////////////////////////////////////////////
    //
    //  Additional BOOST_SPIRIT_DEBUG_FLAGS control the level of diagnostics printed
    //  Basic constants are defined in debug/minimal.hpp.
    //
    ///////////////////////////////////////////////////////////////////////////
    #define POTHOS_SPIRIT_DEBUG_FLAGS_NODES        0x0001  // node diagnostics
    #define POTHOS_SPIRIT_DEBUG_FLAGS_ESCAPE_CHAR  0x0002  // escape_char_parse diagnostics
    #define POTHOS_SPIRIT_DEBUG_FLAGS_TREES        0x0004  // parse tree/ast diagnostics
    #define POTHOS_SPIRIT_DEBUG_FLAGS_CLOSURES     0x0008  // closure diagnostics
    #define POTHOS_SPIRIT_DEBUG_FLAGS_SLEX         0x8000  // slex diagnostics

    #define POTHOS_SPIRIT_DEBUG_FLAGS_MAX          0xFFFF  // print maximal diagnostics

    #if !defined(POTHOS_SPIRIT_DEBUG_FLAGS)
    #define POTHOS_SPIRIT_DEBUG_FLAGS POTHOS_SPIRIT_DEBUG_FLAGS_MAX
    #endif

    ///////////////////////////////////////////////////////////////////////////
    //
    //  By default all nodes are traced (even those, not registered with
    //  BOOST_SPIRIT_DEBUG_RULE et.al. - see below). The following constant may be
    //  used to redefine this default.
    //
    ///////////////////////////////////////////////////////////////////////////
    #if !defined(POTHOS_SPIRIT_DEBUG_TRACENODE)
    #define POTHOS_SPIRIT_DEBUG_TRACENODE          (true)
    #endif // !defined(BOOST_SPIRIT_DEBUG_TRACENODE)

    ///////////////////////////////////////////////////////////////////////////
    //
    //  Helper macros for giving rules and subrules a name accessible through
    //  parser_name() functions (see parser_names.hpp).
    //
    //  Additionally, the macros BOOST_SPIRIT_DEBUG_RULE, SPIRIT_DEBUG_NODE and
    //  BOOST_SPIRIT_DEBUG_GRAMMAR enable/disable the tracing of the 
    //  correspondingnode accordingly to the PP constant 
    //  BOOST_SPIRIT_DEBUG_TRACENODE.
    //
    //  The macros BOOST_SPIRIT_DEBUG_TRACE_RULE, BOOST_SPIRIT_DEBUG_TRACE_NODE 
    //  and BOOST_SPIRIT_DEBUG_TRACE_GRAMMAR allow to specify a flag to define, 
    //  whether the corresponding node is to be traced or not.
    //
    ///////////////////////////////////////////////////////////////////////////
    #if !defined(POTHOS_SPIRIT_DEBUG_RULE)
    #define POTHOS_SPIRIT_DEBUG_RULE(r)    \
        ::POTHOS_SPIRIT_CLASSIC_NS::impl::get_node_registry(). \
            register_node(&r, #r, POTHOS_SPIRIT_DEBUG_TRACENODE)
    #endif // !defined(BOOST_SPIRIT_DEBUG_RULE)

    #if !defined(POTHOS_SPIRIT_DEBUG_NODE)
    #define POTHOS_SPIRIT_DEBUG_NODE(r)    \
        ::POTHOS_SPIRIT_CLASSIC_NS::impl::get_node_registry(). \
            register_node(&r, #r, POTHOS_SPIRIT_DEBUG_TRACENODE)
    #endif // !defined(BOOST_SPIRIT_DEBUG_NODE)

    #if !defined(POTHOS_SPIRIT_DEBUG_GRAMMAR)
    #define POTHOS_SPIRIT_DEBUG_GRAMMAR(r)    \
        ::POTHOS_SPIRIT_CLASSIC_NS::impl::get_node_registry(). \
            register_node(&r, #r, POTHOS_SPIRIT_DEBUG_TRACENODE)
    #endif // !defined(BOOST_SPIRIT_DEBUG_GRAMMAR)

    #if !defined(POTHOS_SPIRIT_DEBUG_TRACE_RULE)
    #define POTHOS_SPIRIT_DEBUG_TRACE_RULE(r, t)    \
        ::POTHOS_SPIRIT_CLASSIC_NS::impl::get_node_registry(). \
            register_node(&r, #r, (t))
    #endif // !defined(BOOST_SPIRIT_TRACE_RULE)

    #if !defined(POTHOS_SPIRIT_DEBUG_TRACE_NODE)
    #define POTHOS_SPIRIT_DEBUG_TRACE_NODE(r, t)    \
        ::POTHOS_SPIRIT_CLASSIC_NS::impl::get_node_registry(). \
            register_node(&r, #r, (t))
    #endif // !defined(BOOST_SPIRIT_DEBUG_TRACE_NODE)

    #if !defined(POTHOS_SPIRIT_DEBUG_TRACE_GRAMMAR)
    #define POTHOS_SPIRIT_DEBUG_TRACE_GRAMMAR(r, t)    \
        ::POTHOS_SPIRIT_CLASSIC_NS::impl::get_node_registry(). \
            register_node(&r, #r, (t))
    #endif // !defined(BOOST_SPIRIT_DEBUG_TRACE_GRAMMAR)

    #if !defined(POTHOS_SPIRIT_DEBUG_TRACE_RULE_NAME)
    #define POTHOS_SPIRIT_DEBUG_TRACE_RULE_NAME(r, n, t)    \
        ::POTHOS_SPIRIT_CLASSIC_NS::impl::get_node_registry(). \
            register_node(&r, (n), (t))
    #endif // !defined(BOOST_SPIRIT_DEBUG_TRACE_RULE_NAME)

    #if !defined(POTHOS_SPIRIT_DEBUG_TRACE_NODE_NAME)
    #define POTHOS_SPIRIT_DEBUG_TRACE_NODE_NAME(r, n, t)    \
        ::POTHOS_SPIRIT_CLASSIC_NS::impl::get_node_registry(). \
            register_node(&r, (n), (t))
    #endif // !defined(BOOST_SPIRIT_DEBUG_TRACE_NODE_NAME)

    #if !defined(POTHOS_SPIRIT_DEBUG_TRACE_GRAMMAR_NAME)
    #define POTHOS_SPIRIT_DEBUG_TRACE_GRAMMAR_NAME(r, n, t)    \
        ::POTHOS_SPIRIT_CLASSIC_NS::impl::get_node_registry(). \
            register_node(&r, (n), (t))
    #endif // !defined(BOOST_SPIRIT_DEBUG_TRACE_GRAMMAR_NAME)

    //////////////////////////////////
    #include <Pothos/serialization/impl/spirit/home/classic/debug/debug_node.hpp>

#else
    //////////////////////////////////
    #include <Pothos/serialization/impl/spirit/home/classic/debug/minimal.hpp>

#endif // BOOST_SPIRIT_DEBUG

#endif

