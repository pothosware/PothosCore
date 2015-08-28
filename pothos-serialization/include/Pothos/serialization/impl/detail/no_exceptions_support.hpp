#ifndef POTHOS_DETAIL_NO_EXCEPTIONS_SUPPORT_HPP_
#define POTHOS_DETAIL_NO_EXCEPTIONS_SUPPORT_HPP_

#if (defined _MSC_VER) && (_MSC_VER >= 1200)
#  pragma once
#endif

//----------------------------------------------------------------------
// (C) Copyright 2004 Pavel Vozenilek.
// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)
//
//
// This file contains helper macros used when exception support may be
// disabled (as indicated by macro BOOST_NO_EXCEPTIONS).
//
// Before picking up these macros you may consider using RAII techniques
// to deal with exceptions - their syntax can be always the same with 
// or without exception support enabled.
//

/* Example of use:

void foo() {
  POTHOS_TRY {
    ...
  } POTHOS_CATCH(const std::bad_alloc&) {
      ...
      POTHOS_RETHROW
  } POTHOS_CATCH(const std::exception& e) {
      ...
  }
  POTHOS_CATCH_END
}

With exception support enabled it will expand into:

void foo() {
  { try {
    ...
  } catch (const std::bad_alloc&) {
      ...
      throw;
  } catch (const std::exception& e) {
      ...
  }
  }
}

With exception support disabled it will expand into:

void foo() {
  { if(true) {
    ...
  } else if (false) {
      ...
  } else if (false)  {
      ...
  }
  }
}
*/
//----------------------------------------------------------------------

#include <Pothos/serialization/impl/config.hpp>
#include <Pothos/serialization/impl/detail/workaround.hpp>

#if !(defined POTHOS_NO_EXCEPTIONS)
#    define POTHOS_TRY { try
#    define POTHOS_CATCH(x) catch(x)
#    define POTHOS_RETHROW throw;
#    define POTHOS_CATCH_END }
#else
#    if POTHOS_WORKAROUND(__BORLANDC__, POTHOS_TESTED_AT(0x564))
#        define POTHOS_TRY { if ("")
#        define POTHOS_CATCH(x) else if (!"")
#    else
#        define POTHOS_TRY { if (true)
#        define POTHOS_CATCH(x) else if (false)
#    endif
#    define POTHOS_RETHROW
#    define POTHOS_CATCH_END }
#endif


#endif 
