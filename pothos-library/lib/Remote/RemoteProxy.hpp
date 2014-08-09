// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Proxy.hpp>
#include <Pothos/Object/Containers.hpp>
#include <mutex>

class RemoteProxyHandle;

/***********************************************************************
 * custom remote environment overload
 **********************************************************************/
class RemoteProxyEnvironment :
    public Pothos::ProxyEnvironment
{
public:
    RemoteProxyEnvironment(std::istream &is, std::ostream &os,
        const std::string &name, const Pothos::ProxyEnvironmentArgs &args);

    ~RemoteProxyEnvironment(void);

    Pothos::Proxy makeHandle(const size_t remoteID);

    std::shared_ptr<RemoteProxyHandle> getHandle(const Pothos::Proxy &proxy);

    std::string getNodeId(void) const
    {
        return nodeId;
    }

    std::string getUniquePid(void) const
    {
        return upid;
    }

    std::string getName(void) const
    {
        return name;
    }

    Pothos::Proxy findProxy(const std::string &name);

    Pothos::Proxy convertObjectToProxy(const Pothos::Object &local);

    Pothos::Object convertProxyToObject(const Pothos::Proxy &proxy);

    void serialize(const Pothos::Proxy &, std::ostream &)
    {
        throw Pothos::ProxySerializeError("RemoteProxyEnvironment::serialize()", "not supported");
    }

    Pothos::Proxy deserialize(std::istream &)
    {
        throw Pothos::ProxySerializeError("RemoteProxyEnvironment::deserialize()", "not supported");
    }

    Pothos::ObjectKwargs transact(const Pothos::ObjectKwargs &request);

    size_t remoteID;
    std::string upid;
    std::string nodeId;

    std::istream &is;
    std::ostream &os;
    const std::string name;
    std::mutex mutex;
    bool connectionActive;
};

/***********************************************************************
 * custom remote class handler overload
 **********************************************************************/
class RemoteProxyHandle : public Pothos::ProxyHandle
{
public:

    RemoteProxyHandle(std::shared_ptr<RemoteProxyEnvironment> env, const size_t remoteID);

    ~RemoteProxyHandle(void);

    Pothos::ProxyEnvironment::Sptr getEnvironment(void) const
    {
        return env;
    }

    Pothos::Proxy call(const std::string &name, const Pothos::Proxy *args, const size_t numArgs);

    int compareTo(const Pothos::Proxy &proxy) const;
    size_t hashCode(void) const;
    std::string toString(void) const;
    std::string getClassName(void) const;

    std::shared_ptr<RemoteProxyEnvironment> env;

    size_t remoteID;
};
