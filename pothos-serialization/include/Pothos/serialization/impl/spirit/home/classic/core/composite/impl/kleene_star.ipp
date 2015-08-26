/*=============================================================================
    Copyright (c) 1998-2003 Joel de Guzman
    Copyright (c) 2001 Daniel Nuffer
    Copyright (c) 2002 Hartmut Kaiser
    http://spirit.sourceforge.net/

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#if !defined(POTHOS_SPIRIT_KLEENE_STAR_IPP)
#define POTHOS_SPIRIT_KLEENE_STAR_IPP

namespace Pothos { namespace spirit {

POTHOS_SPIRIT_CLASSIC_NAMESPACE_BEGIN

    ///////////////////////////////////////////////////////////////////////////
    //
    //  kleene_star class implementation
    //
    ///////////////////////////////////////////////////////////////////////////
    template <typename S>
    inline kleene_star<S>
    operator*(parser<S> const& a)
    {
        return kleene_star<S>(a.derived());
    }

POTHOS_SPIRIT_CLASSIC_NAMESPACE_END

}} // namespace boost::spirit

#endif
