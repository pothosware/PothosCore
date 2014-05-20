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

#for $NARGS in range($MAX_ARGS)
template <typename ReturnType, $expand('typename A%d', $NARGS)>
ReturnType Proxy::call(const std::string &name, $expand('const A%d &a%d', $NARGS)) const
{
    Proxy args[$(max(1, $NARGS))];
    #for $i in range($NARGS):
    args[$i] = Detail::makeProxy(this->getEnvironment(), a$i);
    #end for
    auto handle = this->getHandle();
    assert(handle);
    Proxy ret = handle->call(name, args, $NARGS);
    return Detail::convertProxy<ReturnType>(ret);
}

#cond $NARGS > 0
template <$expand('typename A%d', $NARGS)>
Proxy Proxy::callProxy(const std::string &name, $expand('const A%d &a%d', $NARGS)) const
{
    return this->call<Proxy, $expand('A%d', $NARGS)>(name, $expand('a%d', $NARGS));
}

template <$expand('typename A%d', $NARGS)>
void Proxy::call(const std::string &name, $expand('const A%d &a%d', $NARGS)) const
{
    this->call<Proxy, $expand('A%d', $NARGS)>(name, $expand('a%d', $NARGS));
}
#end if

#end for

} //namespace Pothos
