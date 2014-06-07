// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Proxy.hpp>

class ManagedProxyHandle;

/***********************************************************************
 * custom managed environment overload
 **********************************************************************/
class ManagedProxyEnvironment :
    public Pothos::ProxyEnvironment
{
public:
    ManagedProxyEnvironment(const Pothos::ProxyEnvironmentArgs &);

    Pothos::Proxy makeHandle(const Pothos::Object &obj);

    std::shared_ptr<ManagedProxyHandle> getHandle(const Pothos::Proxy &proxy);

    std::string getName(void) const
    {
        return "managed";
    }

    Pothos::Proxy findProxy(const std::string &name);

    Pothos::Proxy convertObjectToProxy(const Pothos::Object &local);

    Pothos::Object convertProxyToObject(const Pothos::Proxy &proxy);

    void serialize(const Pothos::Proxy &, std::ostream &);

    Pothos::Proxy deserialize(std::istream &);
};

/***********************************************************************
 * custom managed class handler overload
 **********************************************************************/
class ManagedProxyHandle : public Pothos::ProxyHandle
{
public:

    ManagedProxyHandle(std::shared_ptr<ManagedProxyEnvironment> env, const Pothos::Object &obj);

    Pothos::ProxyEnvironment::Sptr getEnvironment(void) const
    {
        return env;
    }

    Pothos::Proxy call(const std::string &name, const Pothos::Proxy *args, const size_t numArgs);

    int compareTo(const Pothos::Proxy &proxy) const;
    size_t hashCode(void) const
    {
        return obj.hashCode();
    }
    std::string toString(void) const;
    std::string getClassName(void) const;

    std::shared_ptr<ManagedProxyEnvironment> env;

    Pothos::Object obj;
};
