// Copyright (c) 2001 Ronald Garcia, Indiana University (garcia@osl.iu.edu)
// Andrew Lumsdaine, Indiana University (lums@osl.iu.edu). Permission to copy, 
// use, modify, sell and distribute this software is granted provided this
// copyright notice appears in all copies. This software is provided "as is"
// without express or implied warranty, and with no claim as to its suitability
// for any purpose.

#ifndef POTHOS_PROGRAM_OPTIONS_UTF8_CODECVT_FACET_HPP
#define POTHOS_PROGRAM_OPTIONS_UTF8_CODECVT_FACET_HPP

#include <Pothos/serialization/impl/program_options/config.hpp>

#define POTHOS_UTF8_BEGIN_NAMESPACE \
     namespace Pothos { namespace program_options { namespace detail {

#define POTHOS_UTF8_END_NAMESPACE }}}
#define POTHOS_UTF8_DECL POTHOS_PROGRAM_OPTIONS_DECL

#include <Pothos/serialization/impl/detail/utf8_codecvt_facet.hpp>

#undef POTHOS_UTF8_BEGIN_NAMESPACE
#undef POTHOS_UTF8_END_NAMESPACE
#undef POTHOS_UTF8_DECL

#endif
