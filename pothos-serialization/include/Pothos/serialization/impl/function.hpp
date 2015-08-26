// Boost.Function library

//  Copyright Douglas Gregor 2001-2003. Use, modification and
//  distribution is subject to the Boost Software License, Version
//  1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

// For more information, see http://www.boost.org/libs/function

// William Kempf, Jesse Jones and Karl Nelson were all very helpful in the
// design of this library.

#include <functional> // unary_function, binary_function

#include <Pothos/serialization/impl/preprocessor/iterate.hpp>
#include <Pothos/serialization/impl/detail/workaround.hpp>

#ifndef POTHOS_FUNCTION_MAX_ARGS
#  define POTHOS_FUNCTION_MAX_ARGS 10
#endif // BOOST_FUNCTION_MAX_ARGS

// Include the prologue here so that the use of file-level iteration
// in anything that may be included by function_template.hpp doesn't break
#include <Pothos/serialization/impl/function/detail/prologue.hpp>

// Older Visual Age C++ version do not handle the file iteration well
#if POTHOS_WORKAROUND(__IBMCPP__, >= 500) && POTHOS_WORKAROUND(__IBMCPP__, < 800)
#  if POTHOS_FUNCTION_MAX_ARGS >= 0
#    include <Pothos/serialization/impl/function/function0.hpp>
#  endif
#  if POTHOS_FUNCTION_MAX_ARGS >= 1
#    include <Pothos/serialization/impl/function/function1.hpp>
#  endif
#  if POTHOS_FUNCTION_MAX_ARGS >= 2
#    include <Pothos/serialization/impl/function/function2.hpp>
#  endif
#  if POTHOS_FUNCTION_MAX_ARGS >= 3
#    include <Pothos/serialization/impl/function/function3.hpp>
#  endif
#  if POTHOS_FUNCTION_MAX_ARGS >= 4
#    include <Pothos/serialization/impl/function/function4.hpp>
#  endif
#  if POTHOS_FUNCTION_MAX_ARGS >= 5
#    include <Pothos/serialization/impl/function/function5.hpp>
#  endif
#  if POTHOS_FUNCTION_MAX_ARGS >= 6
#    include <Pothos/serialization/impl/function/function6.hpp>
#  endif
#  if POTHOS_FUNCTION_MAX_ARGS >= 7
#    include <Pothos/serialization/impl/function/function7.hpp>
#  endif
#  if POTHOS_FUNCTION_MAX_ARGS >= 8
#    include <Pothos/serialization/impl/function/function8.hpp>
#  endif
#  if POTHOS_FUNCTION_MAX_ARGS >= 9
#    include <Pothos/serialization/impl/function/function9.hpp>
#  endif
#  if POTHOS_FUNCTION_MAX_ARGS >= 10
#    include <Pothos/serialization/impl/function/function10.hpp>
#  endif
#else
// What is the '3' for?
#  define POTHOS_PP_ITERATION_PARAMS_1 (3,(0,POTHOS_FUNCTION_MAX_ARGS,<Pothos/serialization/impl/function/detail/function_iterate.hpp>))
#  include POTHOS_PP_ITERATE()
#  undef POTHOS_PP_ITERATION_PARAMS_1
#endif
