// Copyright Vladimir Prus 2004.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#define POTHOS_PROGRAM_OPTIONS_SOURCE
#include <Pothos/serialization/impl/program_options/config.hpp>

#define POTHOS_UTF8_BEGIN_NAMESPACE \
     namespace Pothos { namespace program_options { namespace detail {

#define POTHOS_UTF8_END_NAMESPACE }}}
#define POTHOS_UTF8_DECL POTHOS_PROGRAM_OPTIONS_DECL

#include <Pothos/serialization/impl/detail/utf8_codecvt_facet.ipp>


#undef POTHOS_UTF8_BEGIN_NAMESPACE
#undef POTHOS_UTF8_END_NAMESPACE
#undef POTHOS_UTF8_DECL

