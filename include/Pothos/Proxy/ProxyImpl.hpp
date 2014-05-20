//
// Proxy/ProxyImpl.hpp
//
// Proxy template method implementations.
//
// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0
//

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

} //namespace Detail

template <typename ReturnType>
ReturnType Proxy::call(const std::string &name) const
{
    Proxy args[1];
    auto handle = this->getHandle();
    assert(handle);
    Proxy ret = handle->call(name, args, 0);
    return Detail::convertProxy<ReturnType>(ret);
}


template <typename ReturnType, typename A0>
ReturnType Proxy::call(const std::string &name, const A0 &a0) const
{
    Proxy args[1];
    args[0] = Detail::makeProxy(this->getEnvironment(), a0);
    auto handle = this->getHandle();
    assert(handle);
    Proxy ret = handle->call(name, args, 1);
    return Detail::convertProxy<ReturnType>(ret);
}

template <typename A0>
Proxy Proxy::callProxy(const std::string &name, const A0 &a0) const
{
    return this->call<Proxy, A0>(name, a0);
}

template <typename A0>
void Proxy::call(const std::string &name, const A0 &a0) const
{
    this->call<Proxy, A0>(name, a0);
}

template <typename ReturnType, typename A0, typename A1>
ReturnType Proxy::call(const std::string &name, const A0 &a0, const A1 &a1) const
{
    Proxy args[2];
    args[0] = Detail::makeProxy(this->getEnvironment(), a0);
    args[1] = Detail::makeProxy(this->getEnvironment(), a1);
    auto handle = this->getHandle();
    assert(handle);
    Proxy ret = handle->call(name, args, 2);
    return Detail::convertProxy<ReturnType>(ret);
}

template <typename A0, typename A1>
Proxy Proxy::callProxy(const std::string &name, const A0 &a0, const A1 &a1) const
{
    return this->call<Proxy, A0, A1>(name, a0, a1);
}

template <typename A0, typename A1>
void Proxy::call(const std::string &name, const A0 &a0, const A1 &a1) const
{
    this->call<Proxy, A0, A1>(name, a0, a1);
}

template <typename ReturnType, typename A0, typename A1, typename A2>
ReturnType Proxy::call(const std::string &name, const A0 &a0, const A1 &a1, const A2 &a2) const
{
    Proxy args[3];
    args[0] = Detail::makeProxy(this->getEnvironment(), a0);
    args[1] = Detail::makeProxy(this->getEnvironment(), a1);
    args[2] = Detail::makeProxy(this->getEnvironment(), a2);
    auto handle = this->getHandle();
    assert(handle);
    Proxy ret = handle->call(name, args, 3);
    return Detail::convertProxy<ReturnType>(ret);
}

template <typename A0, typename A1, typename A2>
Proxy Proxy::callProxy(const std::string &name, const A0 &a0, const A1 &a1, const A2 &a2) const
{
    return this->call<Proxy, A0, A1, A2>(name, a0, a1, a2);
}

template <typename A0, typename A1, typename A2>
void Proxy::call(const std::string &name, const A0 &a0, const A1 &a1, const A2 &a2) const
{
    this->call<Proxy, A0, A1, A2>(name, a0, a1, a2);
}

template <typename ReturnType, typename A0, typename A1, typename A2, typename A3>
ReturnType Proxy::call(const std::string &name, const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3) const
{
    Proxy args[4];
    args[0] = Detail::makeProxy(this->getEnvironment(), a0);
    args[1] = Detail::makeProxy(this->getEnvironment(), a1);
    args[2] = Detail::makeProxy(this->getEnvironment(), a2);
    args[3] = Detail::makeProxy(this->getEnvironment(), a3);
    auto handle = this->getHandle();
    assert(handle);
    Proxy ret = handle->call(name, args, 4);
    return Detail::convertProxy<ReturnType>(ret);
}

template <typename A0, typename A1, typename A2, typename A3>
Proxy Proxy::callProxy(const std::string &name, const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3) const
{
    return this->call<Proxy, A0, A1, A2, A3>(name, a0, a1, a2, a3);
}

template <typename A0, typename A1, typename A2, typename A3>
void Proxy::call(const std::string &name, const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3) const
{
    this->call<Proxy, A0, A1, A2, A3>(name, a0, a1, a2, a3);
}

template <typename ReturnType, typename A0, typename A1, typename A2, typename A3, typename A4>
ReturnType Proxy::call(const std::string &name, const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4) const
{
    Proxy args[5];
    args[0] = Detail::makeProxy(this->getEnvironment(), a0);
    args[1] = Detail::makeProxy(this->getEnvironment(), a1);
    args[2] = Detail::makeProxy(this->getEnvironment(), a2);
    args[3] = Detail::makeProxy(this->getEnvironment(), a3);
    args[4] = Detail::makeProxy(this->getEnvironment(), a4);
    auto handle = this->getHandle();
    assert(handle);
    Proxy ret = handle->call(name, args, 5);
    return Detail::convertProxy<ReturnType>(ret);
}

template <typename A0, typename A1, typename A2, typename A3, typename A4>
Proxy Proxy::callProxy(const std::string &name, const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4) const
{
    return this->call<Proxy, A0, A1, A2, A3, A4>(name, a0, a1, a2, a3, a4);
}

template <typename A0, typename A1, typename A2, typename A3, typename A4>
void Proxy::call(const std::string &name, const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4) const
{
    this->call<Proxy, A0, A1, A2, A3, A4>(name, a0, a1, a2, a3, a4);
}

template <typename ReturnType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
ReturnType Proxy::call(const std::string &name, const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5) const
{
    Proxy args[6];
    args[0] = Detail::makeProxy(this->getEnvironment(), a0);
    args[1] = Detail::makeProxy(this->getEnvironment(), a1);
    args[2] = Detail::makeProxy(this->getEnvironment(), a2);
    args[3] = Detail::makeProxy(this->getEnvironment(), a3);
    args[4] = Detail::makeProxy(this->getEnvironment(), a4);
    args[5] = Detail::makeProxy(this->getEnvironment(), a5);
    auto handle = this->getHandle();
    assert(handle);
    Proxy ret = handle->call(name, args, 6);
    return Detail::convertProxy<ReturnType>(ret);
}

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
Proxy Proxy::callProxy(const std::string &name, const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5) const
{
    return this->call<Proxy, A0, A1, A2, A3, A4, A5>(name, a0, a1, a2, a3, a4, a5);
}

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
void Proxy::call(const std::string &name, const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5) const
{
    this->call<Proxy, A0, A1, A2, A3, A4, A5>(name, a0, a1, a2, a3, a4, a5);
}

template <typename ReturnType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
ReturnType Proxy::call(const std::string &name, const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6) const
{
    Proxy args[7];
    args[0] = Detail::makeProxy(this->getEnvironment(), a0);
    args[1] = Detail::makeProxy(this->getEnvironment(), a1);
    args[2] = Detail::makeProxy(this->getEnvironment(), a2);
    args[3] = Detail::makeProxy(this->getEnvironment(), a3);
    args[4] = Detail::makeProxy(this->getEnvironment(), a4);
    args[5] = Detail::makeProxy(this->getEnvironment(), a5);
    args[6] = Detail::makeProxy(this->getEnvironment(), a6);
    auto handle = this->getHandle();
    assert(handle);
    Proxy ret = handle->call(name, args, 7);
    return Detail::convertProxy<ReturnType>(ret);
}

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
Proxy Proxy::callProxy(const std::string &name, const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6) const
{
    return this->call<Proxy, A0, A1, A2, A3, A4, A5, A6>(name, a0, a1, a2, a3, a4, a5, a6);
}

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
void Proxy::call(const std::string &name, const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6) const
{
    this->call<Proxy, A0, A1, A2, A3, A4, A5, A6>(name, a0, a1, a2, a3, a4, a5, a6);
}

template <typename ReturnType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
ReturnType Proxy::call(const std::string &name, const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6, const A7 &a7) const
{
    Proxy args[8];
    args[0] = Detail::makeProxy(this->getEnvironment(), a0);
    args[1] = Detail::makeProxy(this->getEnvironment(), a1);
    args[2] = Detail::makeProxy(this->getEnvironment(), a2);
    args[3] = Detail::makeProxy(this->getEnvironment(), a3);
    args[4] = Detail::makeProxy(this->getEnvironment(), a4);
    args[5] = Detail::makeProxy(this->getEnvironment(), a5);
    args[6] = Detail::makeProxy(this->getEnvironment(), a6);
    args[7] = Detail::makeProxy(this->getEnvironment(), a7);
    auto handle = this->getHandle();
    assert(handle);
    Proxy ret = handle->call(name, args, 8);
    return Detail::convertProxy<ReturnType>(ret);
}

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
Proxy Proxy::callProxy(const std::string &name, const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6, const A7 &a7) const
{
    return this->call<Proxy, A0, A1, A2, A3, A4, A5, A6, A7>(name, a0, a1, a2, a3, a4, a5, a6, a7);
}

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
void Proxy::call(const std::string &name, const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6, const A7 &a7) const
{
    this->call<Proxy, A0, A1, A2, A3, A4, A5, A6, A7>(name, a0, a1, a2, a3, a4, a5, a6, a7);
}

template <typename ReturnType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
ReturnType Proxy::call(const std::string &name, const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6, const A7 &a7, const A8 &a8) const
{
    Proxy args[9];
    args[0] = Detail::makeProxy(this->getEnvironment(), a0);
    args[1] = Detail::makeProxy(this->getEnvironment(), a1);
    args[2] = Detail::makeProxy(this->getEnvironment(), a2);
    args[3] = Detail::makeProxy(this->getEnvironment(), a3);
    args[4] = Detail::makeProxy(this->getEnvironment(), a4);
    args[5] = Detail::makeProxy(this->getEnvironment(), a5);
    args[6] = Detail::makeProxy(this->getEnvironment(), a6);
    args[7] = Detail::makeProxy(this->getEnvironment(), a7);
    args[8] = Detail::makeProxy(this->getEnvironment(), a8);
    auto handle = this->getHandle();
    assert(handle);
    Proxy ret = handle->call(name, args, 9);
    return Detail::convertProxy<ReturnType>(ret);
}

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
Proxy Proxy::callProxy(const std::string &name, const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6, const A7 &a7, const A8 &a8) const
{
    return this->call<Proxy, A0, A1, A2, A3, A4, A5, A6, A7, A8>(name, a0, a1, a2, a3, a4, a5, a6, a7, a8);
}

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
void Proxy::call(const std::string &name, const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6, const A7 &a7, const A8 &a8) const
{
    this->call<Proxy, A0, A1, A2, A3, A4, A5, A6, A7, A8>(name, a0, a1, a2, a3, a4, a5, a6, a7, a8);
}


} //namespace Pothos
