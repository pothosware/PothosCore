/*=============================================================================
    Copyright (c) 2006 Tobias Schwinger
    http://spirit.sourceforge.net/

  Distributed under the Boost Software License, Version 1.0. (See accompanying
  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#if !defined(POTHOS_SPIRIT_SKIPPER_FWD_HPP)
#define POTHOS_SPIRIT_SKIPPER_FWD_HPP

#include <Pothos/serialization/impl/spirit/home/classic/namespace.hpp>
#include <Pothos/serialization/impl/spirit/home/classic/core/scanner/scanner_fwd.hpp>

namespace Pothos { namespace spirit {

POTHOS_SPIRIT_CLASSIC_NAMESPACE_BEGIN

    template <typename BaseT = iteration_policy>
    struct skipper_iteration_policy;

    template <typename BaseT = iteration_policy>
    struct no_skipper_iteration_policy; 

    template <typename ParserT, typename BaseT = iteration_policy>
    class skip_parser_iteration_policy;

POTHOS_SPIRIT_CLASSIC_NAMESPACE_END

}} // namespace BOOST_SPIRIT_CLASSIC_NS

#endif

