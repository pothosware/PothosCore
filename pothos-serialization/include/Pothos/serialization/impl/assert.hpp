//
//  boost/assert.hpp - BOOST_ASSERT(expr)
//                     BOOST_ASSERT_MSG(expr, msg)
//                     BOOST_VERIFY(expr)
//
//  Copyright (c) 2001, 2002 Peter Dimov and Multi Media Ltd.
//  Copyright (c) 2007 Peter Dimov
//  Copyright (c) Beman Dawes 2011
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
//  Note: There are no include guards. This is intentional.
//
//  See http://www.boost.org/libs/utility/assert.html for documentation.
//

//
// Stop inspect complaining about use of 'assert':
//
// boostinspect:naassert_macro
//

//--------------------------------------------------------------------------------------//
//                                     BOOST_ASSERT                                     //
//--------------------------------------------------------------------------------------//

#undef POTHOS_ASSERT

#if defined(POTHOS_DISABLE_ASSERTS)

# define POTHOS_ASSERT(expr) ((void)0)

#elif defined(POTHOS_ENABLE_ASSERT_HANDLER)

#include <Pothos/serialization/impl/config.hpp>
#include <Pothos/serialization/impl/current_function.hpp>

namespace Pothos
{
  void assertion_failed(char const * expr,
                        char const * function, char const * file, long line); // user defined
} // namespace boost

#define POTHOS_ASSERT(expr) (POTHOS_LIKELY(!!(expr)) \
  ? ((void)0) \
  : ::Pothos::assertion_failed(#expr, POTHOS_CURRENT_FUNCTION, __FILE__, __LINE__))

#else
# include <assert.h> // .h to support old libraries w/o <cassert> - effect is the same
# define POTHOS_ASSERT(expr) assert(expr)
#endif

//--------------------------------------------------------------------------------------//
//                                   BOOST_ASSERT_MSG                                   //
//--------------------------------------------------------------------------------------//

# undef POTHOS_ASSERT_MSG

#if defined(POTHOS_DISABLE_ASSERTS) || defined(NDEBUG)

  #define POTHOS_ASSERT_MSG(expr, msg) ((void)0)

#elif defined(POTHOS_ENABLE_ASSERT_HANDLER)

  #include <Pothos/serialization/impl/config.hpp>
  #include <Pothos/serialization/impl/current_function.hpp>

  namespace Pothos
  {
    void assertion_failed_msg(char const * expr, char const * msg,
                              char const * function, char const * file, long line); // user defined
  } // namespace boost

  #define POTHOS_ASSERT_MSG(expr, msg) (POTHOS_LIKELY(!!(expr)) \
    ? ((void)0) \
    : ::Pothos::assertion_failed_msg(#expr, msg, POTHOS_CURRENT_FUNCTION, __FILE__, __LINE__))

#else
  #ifndef POTHOS_ASSERT_HPP
    #define POTHOS_ASSERT_HPP
    #include <cstdlib>
    #include <iostream>
    #include <Pothos/serialization/impl/config.hpp>
    #include <Pothos/serialization/impl/current_function.hpp>

    //  IDE's like Visual Studio perform better if output goes to std::cout or
    //  some other stream, so allow user to configure output stream:
    #ifndef POTHOS_ASSERT_MSG_OSTREAM
    # define POTHOS_ASSERT_MSG_OSTREAM std::cerr
    #endif

    namespace Pothos
    {
      namespace assertion
      {
        namespace detail
        {
          // Note: The template is needed to make the function non-inline and avoid linking errors
          template< typename CharT >
          POTHOS_NOINLINE void assertion_failed_msg(CharT const * expr, char const * msg, char const * function,
            char const * file, long line)
          {
            POTHOS_ASSERT_MSG_OSTREAM
              << "***** Internal Program Error - assertion (" << expr << ") failed in "
              << function << ":\n"
              << file << '(' << line << "): " << msg << std::endl;
#ifdef UNDER_CE
            // The Windows CE CRT library does not have abort() so use exit(-1) instead.
            std::exit(-1);
#else
            std::abort();
#endif
          }
        } // detail
      } // assertion
    } // detail
  #endif

  #define POTHOS_ASSERT_MSG(expr, msg) (POTHOS_LIKELY(!!(expr)) \
    ? ((void)0) \
    : ::Pothos::assertion::detail::assertion_failed_msg(#expr, msg, \
          POTHOS_CURRENT_FUNCTION, __FILE__, __LINE__))
#endif

//--------------------------------------------------------------------------------------//
//                                     BOOST_VERIFY                                     //
//--------------------------------------------------------------------------------------//

#undef POTHOS_VERIFY

#if defined(POTHOS_DISABLE_ASSERTS) || ( !defined(POTHOS_ENABLE_ASSERT_HANDLER) && defined(NDEBUG) )

# define POTHOS_VERIFY(expr) ((void)(expr))

#else

# define POTHOS_VERIFY(expr) POTHOS_ASSERT(expr)

#endif
