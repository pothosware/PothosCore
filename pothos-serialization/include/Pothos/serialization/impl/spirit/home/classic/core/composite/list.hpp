/*=============================================================================
    Copyright (c) 1998-2003 Joel de Guzman
    Copyright (c) 2001 Daniel Nuffer
    Copyright (c) 2002 Hartmut Kaiser
    http://spirit.sourceforge.net/

  Distributed under the Boost Software License, Version 1.0. (See accompanying
  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#if !defined(POTHOS_SPIRIT_LIST_HPP)
#define POTHOS_SPIRIT_LIST_HPP

#include <Pothos/serialization/impl/spirit/home/classic/namespace.hpp>
#include <Pothos/serialization/impl/spirit/home/classic/core/parser.hpp>
#include <Pothos/serialization/impl/spirit/home/classic/core/primitives/primitives.hpp>
#include <Pothos/serialization/impl/spirit/home/classic/core/composite/composite.hpp>
#include <Pothos/serialization/impl/spirit/home/classic/meta/as_parser.hpp>

namespace Pothos { namespace spirit {

POTHOS_SPIRIT_CLASSIC_NAMESPACE_BEGIN

    ///////////////////////////////////////////////////////////////////////////
    //
    //  operator% is defined as:
    //  a % b ---> a >> *(b >> a)
    //
    ///////////////////////////////////////////////////////////////////////////
    template <typename A, typename B>
    sequence<A, kleene_star<sequence<B, A> > >
    operator%(parser<A> const& a, parser<B> const& b);
    
    template <typename A>
    sequence<A, kleene_star<sequence<chlit<char>, A> > >
    operator%(parser<A> const& a, char b);
    
    template <typename B>
    sequence<chlit<char>, kleene_star<sequence<B, chlit<char> > > >
    operator%(char a, parser<B> const& b);
    
    template <typename A>
    sequence<A, kleene_star<sequence<strlit<char const*>, A> > >
    operator%(parser<A> const& a, char const* b);
    
    template <typename B>
    sequence<strlit<char const*>,
        kleene_star<sequence<B, strlit<char const*> > > >
    operator%(char const* a, parser<B> const& b);
    
    template <typename A>
    sequence<A, kleene_star<sequence<chlit<wchar_t>, A> > >
    operator%(parser<A> const& a, wchar_t b);
    
    template <typename B>
    sequence<chlit<wchar_t>, kleene_star<sequence<B, chlit<wchar_t> > > >
    operator%(wchar_t a, parser<B> const& b);
    
    template <typename A>
    sequence<A, kleene_star<sequence<strlit<wchar_t const*>, A> > >
    operator%(parser<A> const& a, wchar_t const* b);
    
    template <typename B>
    sequence<strlit<wchar_t const*>,
        kleene_star<sequence<B, strlit<wchar_t const*> > > >
    operator%(wchar_t const* a, parser<B> const& b);

POTHOS_SPIRIT_CLASSIC_NAMESPACE_END

}} // namespace BOOST_SPIRIT_CLASSIC_NS

#endif

#include <Pothos/serialization/impl/spirit/home/classic/core/composite/impl/list.ipp>
