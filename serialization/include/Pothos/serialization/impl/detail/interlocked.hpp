#ifndef POTHOS_DETAIL_INTERLOCKED_HPP_INCLUDED
#define POTHOS_DETAIL_INTERLOCKED_HPP_INCLUDED

//
//  boost/detail/interlocked.hpp
//
//  Copyright 2005 Peter Dimov
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//

#include <Pothos/serialization/impl/config.hpp>

// MS compatible compilers support #pragma once
#ifdef POTHOS_HAS_PRAGMA_ONCE
#pragma once
#endif

#if defined( POTHOS_USE_WINDOWS_H )

# include <windows.h>

# define POTHOS_INTERLOCKED_INCREMENT InterlockedIncrement
# define POTHOS_INTERLOCKED_DECREMENT InterlockedDecrement
# define POTHOS_INTERLOCKED_COMPARE_EXCHANGE InterlockedCompareExchange
# define POTHOS_INTERLOCKED_EXCHANGE InterlockedExchange
# define POTHOS_INTERLOCKED_EXCHANGE_ADD InterlockedExchangeAdd
# define POTHOS_INTERLOCKED_COMPARE_EXCHANGE_POINTER InterlockedCompareExchangePointer
# define POTHOS_INTERLOCKED_EXCHANGE_POINTER InterlockedExchangePointer

#elif defined( POTHOS_USE_INTRIN_H )

#include <intrin.h>

# define POTHOS_INTERLOCKED_INCREMENT _InterlockedIncrement
# define POTHOS_INTERLOCKED_DECREMENT _InterlockedDecrement
# define POTHOS_INTERLOCKED_COMPARE_EXCHANGE _InterlockedCompareExchange
# define POTHOS_INTERLOCKED_EXCHANGE _InterlockedExchange
# define POTHOS_INTERLOCKED_EXCHANGE_ADD _InterlockedExchangeAdd

# if defined(_M_IA64) || defined(_M_AMD64) || defined(__x86_64__) || defined(__x86_64)

#  define POTHOS_INTERLOCKED_COMPARE_EXCHANGE_POINTER _InterlockedCompareExchangePointer
#  define POTHOS_INTERLOCKED_EXCHANGE_POINTER _InterlockedExchangePointer

# else

#  define POTHOS_INTERLOCKED_COMPARE_EXCHANGE_POINTER(dest,exchange,compare) \
    ((void*)POTHOS_INTERLOCKED_COMPARE_EXCHANGE((long volatile*)(dest),(long)(exchange),(long)(compare)))
#  define POTHOS_INTERLOCKED_EXCHANGE_POINTER(dest,exchange) \
    ((void*)POTHOS_INTERLOCKED_EXCHANGE((long volatile*)(dest),(long)(exchange)))

# endif

#elif defined(_WIN32_WCE)

#if _WIN32_WCE >= 0x600

extern "C" long __cdecl _InterlockedIncrement( long volatile * );
extern "C" long __cdecl _InterlockedDecrement( long volatile * );
extern "C" long __cdecl _InterlockedCompareExchange( long volatile *, long, long );
extern "C" long __cdecl _InterlockedExchange( long volatile *, long );
extern "C" long __cdecl _InterlockedExchangeAdd( long volatile *, long );

# define POTHOS_INTERLOCKED_INCREMENT _InterlockedIncrement
# define POTHOS_INTERLOCKED_DECREMENT _InterlockedDecrement
# define POTHOS_INTERLOCKED_COMPARE_EXCHANGE _InterlockedCompareExchange
# define POTHOS_INTERLOCKED_EXCHANGE _InterlockedExchange
# define POTHOS_INTERLOCKED_EXCHANGE_ADD _InterlockedExchangeAdd

#else
// under Windows CE we still have old-style Interlocked* functions

extern "C" long __cdecl InterlockedIncrement( long* );
extern "C" long __cdecl InterlockedDecrement( long* );
extern "C" long __cdecl InterlockedCompareExchange( long*, long, long );
extern "C" long __cdecl InterlockedExchange( long*, long );
extern "C" long __cdecl InterlockedExchangeAdd( long*, long );

# define POTHOS_INTERLOCKED_INCREMENT InterlockedIncrement
# define POTHOS_INTERLOCKED_DECREMENT InterlockedDecrement
# define POTHOS_INTERLOCKED_COMPARE_EXCHANGE InterlockedCompareExchange
# define POTHOS_INTERLOCKED_EXCHANGE InterlockedExchange
# define POTHOS_INTERLOCKED_EXCHANGE_ADD InterlockedExchangeAdd

#endif

# define POTHOS_INTERLOCKED_COMPARE_EXCHANGE_POINTER(dest,exchange,compare) \
    ((void*)POTHOS_INTERLOCKED_COMPARE_EXCHANGE((long*)(dest),(long)(exchange),(long)(compare)))
# define POTHOS_INTERLOCKED_EXCHANGE_POINTER(dest,exchange) \
    ((void*)POTHOS_INTERLOCKED_EXCHANGE((long*)(dest),(long)(exchange)))

#elif defined( POTHOS_MSVC ) || defined( POTHOS_INTEL_WIN )

#if defined( POTHOS_MSVC ) && POTHOS_MSVC >= 1500

#include <intrin.h>

#elif defined( __CLRCALL_PURE_OR_CDECL )

extern "C" long __CLRCALL_PURE_OR_CDECL _InterlockedIncrement( long volatile * );
extern "C" long __CLRCALL_PURE_OR_CDECL _InterlockedDecrement( long volatile * );
extern "C" long __CLRCALL_PURE_OR_CDECL _InterlockedCompareExchange( long volatile *, long, long );
extern "C" long __CLRCALL_PURE_OR_CDECL _InterlockedExchange( long volatile *, long );
extern "C" long __CLRCALL_PURE_OR_CDECL _InterlockedExchangeAdd( long volatile *, long );

#else

extern "C" long __cdecl _InterlockedIncrement( long volatile * );
extern "C" long __cdecl _InterlockedDecrement( long volatile * );
extern "C" long __cdecl _InterlockedCompareExchange( long volatile *, long, long );
extern "C" long __cdecl _InterlockedExchange( long volatile *, long );
extern "C" long __cdecl _InterlockedExchangeAdd( long volatile *, long );

#endif

# if defined(_M_IA64) || defined(_M_AMD64)

extern "C" void* __cdecl _InterlockedCompareExchangePointer( void* volatile *, void*, void* );
extern "C" void* __cdecl _InterlockedExchangePointer( void* volatile *, void* );

#  define POTHOS_INTERLOCKED_COMPARE_EXCHANGE_POINTER _InterlockedCompareExchangePointer
#  define POTHOS_INTERLOCKED_EXCHANGE_POINTER _InterlockedExchangePointer

# else

#  define POTHOS_INTERLOCKED_COMPARE_EXCHANGE_POINTER(dest,exchange,compare) \
    ((void*)POTHOS_INTERLOCKED_COMPARE_EXCHANGE((long volatile*)(dest),(long)(exchange),(long)(compare)))
#  define POTHOS_INTERLOCKED_EXCHANGE_POINTER(dest,exchange) \
    ((void*)POTHOS_INTERLOCKED_EXCHANGE((long volatile*)(dest),(long)(exchange)))

# endif

# define POTHOS_INTERLOCKED_INCREMENT _InterlockedIncrement
# define POTHOS_INTERLOCKED_DECREMENT _InterlockedDecrement
# define POTHOS_INTERLOCKED_COMPARE_EXCHANGE _InterlockedCompareExchange
# define POTHOS_INTERLOCKED_EXCHANGE _InterlockedExchange
# define POTHOS_INTERLOCKED_EXCHANGE_ADD _InterlockedExchangeAdd

// Unlike __MINGW64__, __MINGW64_VERSION_MAJOR is defined by MinGW-w64 for both 32 and 64-bit targets.
#elif defined(__MINGW64_VERSION_MAJOR)

// MinGW-w64 provides intrin.h for both 32 and 64-bit targets.
#include <intrin.h>

# define POTHOS_INTERLOCKED_INCREMENT _InterlockedIncrement
# define POTHOS_INTERLOCKED_DECREMENT _InterlockedDecrement
# define POTHOS_INTERLOCKED_COMPARE_EXCHANGE _InterlockedCompareExchange
# define POTHOS_INTERLOCKED_EXCHANGE _InterlockedExchange
# define POTHOS_INTERLOCKED_EXCHANGE_ADD _InterlockedExchangeAdd
# if defined(__x86_64__) || defined(__x86_64)
#  define POTHOS_INTERLOCKED_COMPARE_EXCHANGE_POINTER _InterlockedCompareExchangePointer
#  define POTHOS_INTERLOCKED_EXCHANGE_POINTER _InterlockedExchangePointer
# else
#  define POTHOS_INTERLOCKED_COMPARE_EXCHANGE_POINTER(dest,exchange,compare) \
    ((void*)POTHOS_INTERLOCKED_COMPARE_EXCHANGE((long volatile*)(dest),(long)(exchange),(long)(compare)))
#  define POTHOS_INTERLOCKED_EXCHANGE_POINTER(dest,exchange) \
    ((void*)POTHOS_INTERLOCKED_EXCHANGE((long volatile*)(dest),(long)(exchange)))
# endif

#elif defined( WIN32 ) || defined( _WIN32 ) || defined( __WIN32__ ) || defined( __CYGWIN__ )

#define POTHOS_INTERLOCKED_IMPORT __declspec(dllimport)

namespace Pothos
{

namespace detail
{

extern "C" POTHOS_INTERLOCKED_IMPORT long __stdcall InterlockedIncrement( long volatile * );
extern "C" POTHOS_INTERLOCKED_IMPORT long __stdcall InterlockedDecrement( long volatile * );
extern "C" POTHOS_INTERLOCKED_IMPORT long __stdcall InterlockedCompareExchange( long volatile *, long, long );
extern "C" POTHOS_INTERLOCKED_IMPORT long __stdcall InterlockedExchange( long volatile *, long );
extern "C" POTHOS_INTERLOCKED_IMPORT long __stdcall InterlockedExchangeAdd( long volatile *, long );

# if defined(_M_IA64) || defined(_M_AMD64)
extern "C" POTHOS_INTERLOCKED_IMPORT void* __stdcall InterlockedCompareExchangePointer( void* volatile *, void*, void* );
extern "C" POTHOS_INTERLOCKED_IMPORT void* __stdcall InterlockedExchangePointer( void* volatile *, void* );
# endif

} // namespace detail

} // namespace boost

# define POTHOS_INTERLOCKED_INCREMENT ::Pothos::detail::InterlockedIncrement
# define POTHOS_INTERLOCKED_DECREMENT ::Pothos::detail::InterlockedDecrement
# define POTHOS_INTERLOCKED_COMPARE_EXCHANGE ::Pothos::detail::InterlockedCompareExchange
# define POTHOS_INTERLOCKED_EXCHANGE ::Pothos::detail::InterlockedExchange
# define POTHOS_INTERLOCKED_EXCHANGE_ADD ::Pothos::detail::InterlockedExchangeAdd

# if defined(_M_IA64) || defined(_M_AMD64)
#  define POTHOS_INTERLOCKED_COMPARE_EXCHANGE_POINTER ::Pothos::detail::InterlockedCompareExchangePointer
#  define POTHOS_INTERLOCKED_EXCHANGE_POINTER ::Pothos::detail::InterlockedExchangePointer
# else
#  define POTHOS_INTERLOCKED_COMPARE_EXCHANGE_POINTER(dest,exchange,compare) \
    ((void*)POTHOS_INTERLOCKED_COMPARE_EXCHANGE((long volatile*)(dest),(long)(exchange),(long)(compare)))
#  define POTHOS_INTERLOCKED_EXCHANGE_POINTER(dest,exchange) \
    ((void*)POTHOS_INTERLOCKED_EXCHANGE((long volatile*)(dest),(long)(exchange)))
# endif

#else

# error "Interlocked intrinsics not available"

#endif

#endif // #ifndef BOOST_DETAIL_INTERLOCKED_HPP_INCLUDED
