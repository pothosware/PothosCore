#ifndef POTHOS_BIND_MEM_FN_HPP_INCLUDED
#define POTHOS_BIND_MEM_FN_HPP_INCLUDED

// MS compatible compilers support #pragma once

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

//
//  mem_fn.hpp - a generalization of std::mem_fun[_ref]
//
//  Copyright (c) 2001, 2002 Peter Dimov and Multi Media Ltd.
//  Copyright (c) 2001 David Abrahams
//  Copyright (c) 2003-2005 Peter Dimov
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
//  See http://www.boost.org/libs/bind/mem_fn.html for documentation.
//

#include <Pothos/serialization/impl/config.hpp>
#include <Pothos/serialization/impl/get_pointer.hpp>
#include <Pothos/serialization/impl/detail/workaround.hpp>

namespace Pothos
{

#if defined(POTHOS_NO_VOID_RETURNS)

#define POTHOS_MEM_FN_CLASS_F , class F
#define POTHOS_MEM_FN_TYPEDEF(X)

namespace _mfi // mem_fun_impl
{

template<class V> struct mf
{

#define POTHOS_MEM_FN_RETURN return

#define POTHOS_MEM_FN_NAME(X) inner_##X
#define POTHOS_MEM_FN_CC

#include <Pothos/serialization/impl/bind/mem_fn_template.hpp>

#undef POTHOS_MEM_FN_CC
#undef POTHOS_MEM_FN_NAME

#ifdef POTHOS_MEM_FN_ENABLE_CDECL

#define POTHOS_MEM_FN_NAME(X) inner_##X##_cdecl
#define POTHOS_MEM_FN_CC __cdecl

#include <Pothos/serialization/impl/bind/mem_fn_template.hpp>

#undef POTHOS_MEM_FN_CC
#undef POTHOS_MEM_FN_NAME

#endif

#ifdef POTHOS_MEM_FN_ENABLE_STDCALL

#define POTHOS_MEM_FN_NAME(X) inner_##X##_stdcall
#define POTHOS_MEM_FN_CC __stdcall

#include <Pothos/serialization/impl/bind/mem_fn_template.hpp>

#undef POTHOS_MEM_FN_CC
#undef POTHOS_MEM_FN_NAME

#endif

#ifdef POTHOS_MEM_FN_ENABLE_FASTCALL

#define POTHOS_MEM_FN_NAME(X) inner_##X##_fastcall
#define POTHOS_MEM_FN_CC __fastcall

#include <Pothos/serialization/impl/bind/mem_fn_template.hpp>

#undef POTHOS_MEM_FN_CC
#undef POTHOS_MEM_FN_NAME

#endif

#undef POTHOS_MEM_FN_RETURN

}; // struct mf<V>

template<> struct mf<void>
{

#define POTHOS_MEM_FN_RETURN

#define POTHOS_MEM_FN_NAME(X) inner_##X
#define POTHOS_MEM_FN_CC

#include <Pothos/serialization/impl/bind/mem_fn_template.hpp>

#undef POTHOS_MEM_FN_CC
#undef POTHOS_MEM_FN_NAME

#ifdef POTHOS_MEM_FN_ENABLE_CDECL

#define POTHOS_MEM_FN_NAME(X) inner_##X##_cdecl
#define POTHOS_MEM_FN_CC __cdecl

#include <Pothos/serialization/impl/bind/mem_fn_template.hpp>

#undef POTHOS_MEM_FN_CC
#undef POTHOS_MEM_FN_NAME

#endif

#ifdef POTHOS_MEM_FN_ENABLE_STDCALL

#define POTHOS_MEM_FN_NAME(X) inner_##X##_stdcall
#define POTHOS_MEM_FN_CC __stdcall

#include <Pothos/serialization/impl/bind/mem_fn_template.hpp>

#undef POTHOS_MEM_FN_CC
#undef POTHOS_MEM_FN_NAME

#endif

#ifdef POTHOS_MEM_FN_ENABLE_FASTCALL

#define POTHOS_MEM_FN_NAME(X) inner_##X##_fastcall
#define POTHOS_MEM_FN_CC __fastcall

#include <Pothos/serialization/impl/bind/mem_fn_template.hpp>

#undef POTHOS_MEM_FN_CC
#undef POTHOS_MEM_FN_NAME

#endif

#undef POTHOS_MEM_FN_RETURN

}; // struct mf<void>

#undef POTHOS_MEM_FN_CLASS_F
#undef POTHOS_MEM_FN_TYPEDEF_F

#define POTHOS_MEM_FN_NAME(X) X
#define POTHOS_MEM_FN_NAME2(X) inner_##X
#define POTHOS_MEM_FN_CC

#include <Pothos/serialization/impl/bind/mem_fn_vw.hpp>

#undef POTHOS_MEM_FN_NAME
#undef POTHOS_MEM_FN_NAME2
#undef POTHOS_MEM_FN_CC

#ifdef POTHOS_MEM_FN_ENABLE_CDECL

#define POTHOS_MEM_FN_NAME(X) X##_cdecl
#define POTHOS_MEM_FN_NAME2(X) inner_##X##_cdecl
#define POTHOS_MEM_FN_CC __cdecl

#include <Pothos/serialization/impl/bind/mem_fn_vw.hpp>

#undef POTHOS_MEM_FN_NAME
#undef POTHOS_MEM_FN_NAME2
#undef POTHOS_MEM_FN_CC

#endif

#ifdef POTHOS_MEM_FN_ENABLE_STDCALL

#define POTHOS_MEM_FN_NAME(X) X##_stdcall
#define POTHOS_MEM_FN_NAME2(X) inner_##X##_stdcall
#define POTHOS_MEM_FN_CC __stdcall

#include <Pothos/serialization/impl/bind/mem_fn_vw.hpp>

#undef POTHOS_MEM_FN_NAME
#undef POTHOS_MEM_FN_NAME2
#undef POTHOS_MEM_FN_CC

#endif

#ifdef POTHOS_MEM_FN_ENABLE_FASTCALL

#define POTHOS_MEM_FN_NAME(X) X##_fastcall
#define POTHOS_MEM_FN_NAME2(X) inner_##X##_fastcall
#define POTHOS_MEM_FN_CC __fastcall

#include <Pothos/serialization/impl/bind/mem_fn_vw.hpp>

#undef POTHOS_MEM_FN_NAME
#undef POTHOS_MEM_FN_NAME2
#undef POTHOS_MEM_FN_CC

#endif

} // namespace _mfi

#else // #ifdef BOOST_NO_VOID_RETURNS

#define POTHOS_MEM_FN_CLASS_F
#define POTHOS_MEM_FN_TYPEDEF(X) typedef X;

namespace _mfi
{

#define POTHOS_MEM_FN_RETURN return

#define POTHOS_MEM_FN_NAME(X) X
#define POTHOS_MEM_FN_CC

#include <Pothos/serialization/impl/bind/mem_fn_template.hpp>

#undef POTHOS_MEM_FN_CC
#undef POTHOS_MEM_FN_NAME

#ifdef POTHOS_MEM_FN_ENABLE_CDECL

#define POTHOS_MEM_FN_NAME(X) X##_cdecl
#define POTHOS_MEM_FN_CC __cdecl

#include <Pothos/serialization/impl/bind/mem_fn_template.hpp>

#undef POTHOS_MEM_FN_CC
#undef POTHOS_MEM_FN_NAME

#endif

#ifdef POTHOS_MEM_FN_ENABLE_STDCALL

#define POTHOS_MEM_FN_NAME(X) X##_stdcall
#define POTHOS_MEM_FN_CC __stdcall

#include <Pothos/serialization/impl/bind/mem_fn_template.hpp>

#undef POTHOS_MEM_FN_CC
#undef POTHOS_MEM_FN_NAME

#endif

#ifdef POTHOS_MEM_FN_ENABLE_FASTCALL

#define POTHOS_MEM_FN_NAME(X) X##_fastcall
#define POTHOS_MEM_FN_CC __fastcall

#include <Pothos/serialization/impl/bind/mem_fn_template.hpp>

#undef POTHOS_MEM_FN_CC
#undef POTHOS_MEM_FN_NAME

#endif

#undef POTHOS_MEM_FN_RETURN

} // namespace _mfi

#undef POTHOS_MEM_FN_CLASS_F
#undef POTHOS_MEM_FN_TYPEDEF

#endif // #ifdef BOOST_NO_VOID_RETURNS

#define POTHOS_MEM_FN_NAME(X) X
#define POTHOS_MEM_FN_CC

#include <Pothos/serialization/impl/bind/mem_fn_cc.hpp>

#undef POTHOS_MEM_FN_NAME
#undef POTHOS_MEM_FN_CC

#ifdef POTHOS_MEM_FN_ENABLE_CDECL

#define POTHOS_MEM_FN_NAME(X) X##_cdecl
#define POTHOS_MEM_FN_CC __cdecl

#include <Pothos/serialization/impl/bind/mem_fn_cc.hpp>

#undef POTHOS_MEM_FN_NAME
#undef POTHOS_MEM_FN_CC

#endif

#ifdef POTHOS_MEM_FN_ENABLE_STDCALL

#define POTHOS_MEM_FN_NAME(X) X##_stdcall
#define POTHOS_MEM_FN_CC __stdcall

#include <Pothos/serialization/impl/bind/mem_fn_cc.hpp>

#undef POTHOS_MEM_FN_NAME
#undef POTHOS_MEM_FN_CC

#endif

#ifdef POTHOS_MEM_FN_ENABLE_FASTCALL

#define POTHOS_MEM_FN_NAME(X) X##_fastcall
#define POTHOS_MEM_FN_CC __fastcall

#include <Pothos/serialization/impl/bind/mem_fn_cc.hpp>

#undef POTHOS_MEM_FN_NAME
#undef POTHOS_MEM_FN_CC

#endif

// data member support

namespace _mfi
{

template<class R, class T> class dm
{
public:

    typedef R const & result_type;
    typedef T const * argument_type;

private:
    
    typedef R (T::*F);
    F f_;

    template<class U> R const & call(U & u, T const *) const
    {
        return (u.*f_);
    }

    template<class U> R const & call(U & u, void const *) const
    {
        return (get_pointer(u)->*f_);
    }

public:
    
    explicit dm(F f): f_(f) {}

    R & operator()(T * p) const
    {
        return (p->*f_);
    }

    R const & operator()(T const * p) const
    {
        return (p->*f_);
    }

    template<class U> R const & operator()(U const & u) const
    {
        return call(u, &u);
    }

#if !POTHOS_WORKAROUND(POTHOS_MSVC, <= 1300) && !POTHOS_WORKAROUND(__MWERKS__, < 0x3200)

    R & operator()(T & t) const
    {
        return (t.*f_);
    }

    R const & operator()(T const & t) const
    {
        return (t.*f_);
    }

#endif

    bool operator==(dm const & rhs) const
    {
        return f_ == rhs.f_;
    }

    bool operator!=(dm const & rhs) const
    {
        return f_ != rhs.f_;
    }
};

} // namespace _mfi

template<class R, class T> _mfi::dm<R, T> mem_fn(R T::*f)
{
    return _mfi::dm<R, T>(f);
}

} // namespace boost

#endif // #ifndef BOOST_BIND_MEM_FN_HPP_INCLUDED
