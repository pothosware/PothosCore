/*=============================================================================
    Copyright (c) 1998-2003 Joel de Guzman
    Copyright (c) 2001 Daniel Nuffer
    Copyright (c) 2002 Hartmut Kaiser
    http://spirit.sourceforge.net/

  Distributed under the Boost Software License, Version 1.0. (See accompanying
  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#if !defined(POTHOS_SPIRIT_SEQUENTIAL_AND_HPP)
#define POTHOS_SPIRIT_SEQUENTIAL_AND_HPP

#include <Pothos/serialization/impl/spirit/home/classic/namespace.hpp>
#include <Pothos/serialization/impl/spirit/home/classic/core/parser.hpp>
#include <Pothos/serialization/impl/spirit/home/classic/core/primitives/primitives.hpp>
#include <Pothos/serialization/impl/spirit/home/classic/core/composite/composite.hpp>
#include <Pothos/serialization/impl/spirit/home/classic/meta/as_parser.hpp>

namespace Pothos { namespace spirit {

POTHOS_SPIRIT_CLASSIC_NAMESPACE_BEGIN

    ///////////////////////////////////////////////////////////////////////////
    //
    //  sequential-and operators
    //
    //      Handles expressions of the form:
    //
    //          a && b
    //
    //      Same as a >> b.
    //
    ///////////////////////////////////////////////////////////////////////////
    template <typename A, typename B>
    sequence<A, B>
    operator&&(parser<A> const& a, parser<B> const& b);
    
    template <typename A>
    sequence<A, chlit<char> >
    operator&&(parser<A> const& a, char b);
    
    template <typename B>
    sequence<chlit<char>, B>
    operator&&(char a, parser<B> const& b);
    
    template <typename A>
    sequence<A, strlit<char const*> >
    operator&&(parser<A> const& a, char const* b);
    
    template <typename B>
    sequence<strlit<char const*>, B>
    operator&&(char const* a, parser<B> const& b);
    
    template <typename A>
    sequence<A, chlit<wchar_t> >
    operator&&(parser<A> const& a, wchar_t b);
    
    template <typename B>
    sequence<chlit<wchar_t>, B>
    operator&&(wchar_t a, parser<B> const& b);
    
    template <typename A>
    sequence<A, strlit<wchar_t const*> >
    operator&&(parser<A> const& a, wchar_t const* b);
    
    template <typename B>
    sequence<strlit<wchar_t const*>, B>
    operator&&(wchar_t const* a, parser<B> const& b);

POTHOS_SPIRIT_CLASSIC_NAMESPACE_END

}} // namespace BOOST_SPIRIT_CLASSIC_NS

#endif

#include <Pothos/serialization/impl/spirit/home/classic/core/composite/impl/sequential_and.ipp>
