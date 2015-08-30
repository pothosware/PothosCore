
//  (C) Copyright John maddock 1999. 
//  (C) David Abrahams 2002.  Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// use this header as a workaround for missing <limits>

//  See http://www.boost.org/libs/compatibility/index.html for documentation.

#ifndef POTHOS_LIMITS
#define POTHOS_LIMITS

#include <Pothos/serialization/impl/config.hpp>

#ifdef POTHOS_NO_LIMITS
#  error "There is no std::numeric_limits suppport available."
#else
# include <limits>
#endif

#if (defined(POTHOS_HAS_LONG_LONG) && defined(POTHOS_NO_LONG_LONG_NUMERIC_LIMITS)) \
      || (defined(POTHOS_HAS_MS_INT64) && defined(POTHOS_NO_MS_INT64_NUMERIC_LIMITS))
// Add missing specializations for numeric_limits:
#ifdef POTHOS_HAS_MS_INT64
#  define POTHOS_LLT __int64
#  define POTHOS_ULLT unsigned __int64
#else
#  define POTHOS_LLT  ::Pothos::long_long_type
#  define POTHOS_ULLT  ::Pothos::ulong_long_type
#endif

#include <climits>  // for CHAR_BIT

namespace std
{
  template<>
  class numeric_limits<POTHOS_LLT> 
  {
   public:

      POTHOS_STATIC_CONSTANT(bool, is_specialized = true);
#ifdef POTHOS_HAS_MS_INT64
      static POTHOS_LLT min POTHOS_PREVENT_MACRO_SUBSTITUTION (){ return 0x8000000000000000i64; }
      static POTHOS_LLT max POTHOS_PREVENT_MACRO_SUBSTITUTION (){ return 0x7FFFFFFFFFFFFFFFi64; }
#elif defined(LLONG_MAX)
      static POTHOS_LLT min POTHOS_PREVENT_MACRO_SUBSTITUTION (){ return LLONG_MIN; }
      static POTHOS_LLT max POTHOS_PREVENT_MACRO_SUBSTITUTION (){ return LLONG_MAX; }
#elif defined(LONGLONG_MAX)
      static POTHOS_LLT min POTHOS_PREVENT_MACRO_SUBSTITUTION (){ return LONGLONG_MIN; }
      static POTHOS_LLT max POTHOS_PREVENT_MACRO_SUBSTITUTION (){ return LONGLONG_MAX; }
#else
      static POTHOS_LLT min POTHOS_PREVENT_MACRO_SUBSTITUTION (){ return 1LL << (sizeof(POTHOS_LLT) * CHAR_BIT - 1); }
      static POTHOS_LLT max POTHOS_PREVENT_MACRO_SUBSTITUTION (){ return ~(min)(); }
#endif
      POTHOS_STATIC_CONSTANT(int, digits = sizeof(POTHOS_LLT) * CHAR_BIT -1);
      POTHOS_STATIC_CONSTANT(int, digits10 = (CHAR_BIT * sizeof (POTHOS_LLT) - 1) * 301L / 1000);
      POTHOS_STATIC_CONSTANT(bool, is_signed = true);
      POTHOS_STATIC_CONSTANT(bool, is_integer = true);
      POTHOS_STATIC_CONSTANT(bool, is_exact = true);
      POTHOS_STATIC_CONSTANT(int, radix = 2);
      static POTHOS_LLT epsilon() throw() { return 0; };
      static POTHOS_LLT round_error() throw() { return 0; };

      POTHOS_STATIC_CONSTANT(int, min_exponent = 0);
      POTHOS_STATIC_CONSTANT(int, min_exponent10 = 0);
      POTHOS_STATIC_CONSTANT(int, max_exponent = 0);
      POTHOS_STATIC_CONSTANT(int, max_exponent10 = 0);

      POTHOS_STATIC_CONSTANT(bool, has_infinity = false);
      POTHOS_STATIC_CONSTANT(bool, has_quiet_NaN = false);
      POTHOS_STATIC_CONSTANT(bool, has_signaling_NaN = false);
      POTHOS_STATIC_CONSTANT(bool, has_denorm = false);
      POTHOS_STATIC_CONSTANT(bool, has_denorm_loss = false);
      static POTHOS_LLT infinity() throw() { return 0; };
      static POTHOS_LLT quiet_NaN() throw() { return 0; };
      static POTHOS_LLT signaling_NaN() throw() { return 0; };
      static POTHOS_LLT denorm_min() throw() { return 0; };

      POTHOS_STATIC_CONSTANT(bool, is_iec559 = false);
      POTHOS_STATIC_CONSTANT(bool, is_bounded = true);
      POTHOS_STATIC_CONSTANT(bool, is_modulo = true);

      POTHOS_STATIC_CONSTANT(bool, traps = false);
      POTHOS_STATIC_CONSTANT(bool, tinyness_before = false);
      POTHOS_STATIC_CONSTANT(float_round_style, round_style = round_toward_zero);
      
  };

  template<>
  class numeric_limits<POTHOS_ULLT> 
  {
   public:

      POTHOS_STATIC_CONSTANT(bool, is_specialized = true);
#ifdef POTHOS_HAS_MS_INT64
      static POTHOS_ULLT min POTHOS_PREVENT_MACRO_SUBSTITUTION (){ return 0ui64; }
      static POTHOS_ULLT max POTHOS_PREVENT_MACRO_SUBSTITUTION (){ return 0xFFFFFFFFFFFFFFFFui64; }
#elif defined(ULLONG_MAX) && defined(ULLONG_MIN)
      static POTHOS_ULLT min POTHOS_PREVENT_MACRO_SUBSTITUTION (){ return ULLONG_MIN; }
      static POTHOS_ULLT max POTHOS_PREVENT_MACRO_SUBSTITUTION (){ return ULLONG_MAX; }
#elif defined(ULONGLONG_MAX) && defined(ULONGLONG_MIN)
      static POTHOS_ULLT min POTHOS_PREVENT_MACRO_SUBSTITUTION (){ return ULONGLONG_MIN; }
      static POTHOS_ULLT max POTHOS_PREVENT_MACRO_SUBSTITUTION (){ return ULONGLONG_MAX; }
#else
      static POTHOS_ULLT min POTHOS_PREVENT_MACRO_SUBSTITUTION (){ return 0uLL; }
      static POTHOS_ULLT max POTHOS_PREVENT_MACRO_SUBSTITUTION (){ return ~0uLL; }
#endif
      POTHOS_STATIC_CONSTANT(int, digits = sizeof(POTHOS_LLT) * CHAR_BIT);
      POTHOS_STATIC_CONSTANT(int, digits10 = (CHAR_BIT * sizeof (POTHOS_LLT)) * 301L / 1000);
      POTHOS_STATIC_CONSTANT(bool, is_signed = false);
      POTHOS_STATIC_CONSTANT(bool, is_integer = true);
      POTHOS_STATIC_CONSTANT(bool, is_exact = true);
      POTHOS_STATIC_CONSTANT(int, radix = 2);
      static POTHOS_ULLT epsilon() throw() { return 0; };
      static POTHOS_ULLT round_error() throw() { return 0; };

      POTHOS_STATIC_CONSTANT(int, min_exponent = 0);
      POTHOS_STATIC_CONSTANT(int, min_exponent10 = 0);
      POTHOS_STATIC_CONSTANT(int, max_exponent = 0);
      POTHOS_STATIC_CONSTANT(int, max_exponent10 = 0);

      POTHOS_STATIC_CONSTANT(bool, has_infinity = false);
      POTHOS_STATIC_CONSTANT(bool, has_quiet_NaN = false);
      POTHOS_STATIC_CONSTANT(bool, has_signaling_NaN = false);
      POTHOS_STATIC_CONSTANT(bool, has_denorm = false);
      POTHOS_STATIC_CONSTANT(bool, has_denorm_loss = false);
      static POTHOS_ULLT infinity() throw() { return 0; };
      static POTHOS_ULLT quiet_NaN() throw() { return 0; };
      static POTHOS_ULLT signaling_NaN() throw() { return 0; };
      static POTHOS_ULLT denorm_min() throw() { return 0; };

      POTHOS_STATIC_CONSTANT(bool, is_iec559 = false);
      POTHOS_STATIC_CONSTANT(bool, is_bounded = true);
      POTHOS_STATIC_CONSTANT(bool, is_modulo = true);

      POTHOS_STATIC_CONSTANT(bool, traps = false);
      POTHOS_STATIC_CONSTANT(bool, tinyness_before = false);
      POTHOS_STATIC_CONSTANT(float_round_style, round_style = round_toward_zero);
      
  };
}
#endif 

#endif

