///
/// \file Proxy/ProxyImpl.hpp
///
/// Proxy template method implementations.
///
/// \copyright
/// Copyright (c) 2013-2016 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Object/ObjectImpl.hpp>
#include <Pothos/Proxy/Proxy.hpp>
#include <Pothos/Proxy/Handle.hpp>
#include <Pothos/Proxy/Environment.hpp>
#include <type_traits> //enable_if
#include <cassert>

namespace Pothos {

template <typename ValueType>
ValueType Proxy::convert(void) const
{
    return this->getEnvironment()->convertProxyToObject(*this).convert<ValueType>();
}

namespace Detail {

/***********************************************************************
 * convertProxy either converts a proxy to a desired type
 * or returns a proxy if the requested type was a proxy
 **********************************************************************/
template <typename T>
typename std::enable_if<!std::is_same<T, Proxy>::value, T>::type
convertProxy(const Proxy &p)
{
    return p.convert<T>();
}

template <typename T>
typename std::enable_if<std::is_same<T, Proxy>::value, T>::type
convertProxy(const Proxy &p)
{
    return p;
}

/***********************************************************************
 * makeProxy either makes a proxy from an arbitrary type
 * or returns a proxy if the type is already a proxy
 **********************************************************************/
template <typename T>
Proxy makeProxy(const ProxyEnvironment::Sptr &env, const T &value)
{
    return env->makeProxy(value);
}

inline Proxy makeProxy(const ProxyEnvironment::Sptr &, const Proxy &value)
{
    return value;
}

//! The empty arguments tail case
inline void loadProxyArgs(Proxy *, const ProxyEnvironment::Sptr &)
{
    return;
}

//! Recurse to fill the output with each arg
template <typename Arg0Type, typename... ArgsType>
void loadProxyArgs(Proxy *out, const ProxyEnvironment::Sptr &env, const Arg0Type &a0, const ArgsType&... args)
{
    *out++ = makeProxy(env, a0);
    loadProxyArgs(out, env, args...);
}

} //namespace Detail

template <typename ReturnType, typename... ArgsType>
ReturnType Proxy::call(const std::string &name, const ArgsType&... args) const
{
    Proxy proxyArgs[sizeof...(args)];
    Detail::loadProxyArgs(proxyArgs, this->getEnvironment(), args...);
    auto handle = this->getHandle();
    assert(handle);
    Proxy ret = handle->call(name, proxyArgs, sizeof...(args));
    return Detail::convertProxy<ReturnType>(ret);
}

template <typename... ArgsType>
Proxy Proxy::callProxy(const std::string &name, const ArgsType&... args) const
{
    return this->call<Proxy, ArgsType...>(name, args...);
}

template <typename... ArgsType>
void Proxy::callVoid(const std::string &name, const ArgsType&... args) const
{
    this->call<Proxy, ArgsType...>(name, args...);
}

} //namespace Pothos
