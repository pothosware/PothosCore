// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "Remote/RemoteProxy.hpp"
#include <Poco/Format.h>
#include <Poco/Logger.h>
#include <iostream>

RemoteProxyHandle::RemoteProxyHandle(std::shared_ptr<RemoteProxyEnvironment> env, const size_t remoteID):
    env(env), remoteID(remoteID)
{
    return;
}

RemoteProxyHandle::~RemoteProxyHandle(void)
{
    //create request
    Pothos::ObjectKwargs req;
    req["action"] = Pothos::Object("~RemoteProxyHandle");
    req["handleID"] = Pothos::Object(this->remoteID);

    try
    {
        env->transact(req);
    }
    catch(const Pothos::Exception &ex)
    {
        poco_error(Poco::Logger::get("Pothos.RemoteProxyHandle"), "destructor threw: "+ex.displayText());
    }
}

Pothos::Proxy RemoteProxyHandle::call(const std::string &name, const Pothos::Proxy *args, const size_t numArgs)
{
    //create request
    Pothos::ObjectKwargs req;
    req["action"] = Pothos::Object("call");
    req["handleID"] = Pothos::Object(this->remoteID);
    req["name"] = Pothos::Object(name);
    for (size_t i = 0; i < numArgs; i++)
    {
        std::shared_ptr<RemoteProxyHandle> handle;
        try
        {
            handle = env->getHandle(args[i]);
        }
        catch(const std::exception &ex)
        {
            throw Pothos::ProxyHandleCallError("RemoteProxyHandle::call("+name+")",
                Poco::format("convert arg %d - %s", int(i), std::string(ex.what())));
        }
        req[std::to_string(i)] = Pothos::Object(handle->remoteID);
    }

    auto reply = env->transact(req);

    //check for an error
    auto errorMsgIt = reply.find("errorMsg");
    if (errorMsgIt != reply.end()) throw Pothos::ProxyHandleCallError(
        "RemoteProxyEnvironment::call("+name+")", errorMsgIt->second.extract<std::string>());

    //check for a message
    auto messageIt = reply.find("message");
    if (messageIt != reply.end()) throw Pothos::ProxyExceptionMessage(messageIt->second.extract<std::string>());

    //otherwise make a handle
    return env->makeHandle(reply["handleID"].convert<size_t>());
}

int RemoteProxyHandle::compareTo(const Pothos::Proxy &proxy) const
{
    std::shared_ptr<RemoteProxyHandle> handle;
    try
    {
        handle = env->getHandle(proxy);
    }
    catch(const std::exception &ex)
    {
        throw Pothos::ProxyCompareError("RemoteProxyHandle::compareTo()", Poco::format(
            "convert %s to remote - %s", proxy.toString(), std::string(ex.what())));
    }

    //create request
    Pothos::ObjectKwargs req;
    req["action"] = Pothos::Object("compareTo");
    req["handleID"] = Pothos::Object(this->remoteID);
    req["otherID"] = Pothos::Object(handle->remoteID);

    auto reply = env->transact(req);

    //check for an error
    auto errorMsgIt = reply.find("errorMsg");
    if (errorMsgIt != reply.end()) throw Pothos::ProxyCompareError(
        "RemoteProxyEnvironment::compareTo()", errorMsgIt->second.extract<std::string>());

    return reply["result"].convert<int>();
}

size_t RemoteProxyHandle::hashCode(void) const
{
    //create request
    Pothos::ObjectKwargs req;
    req["action"] = Pothos::Object("hashCode");
    req["handleID"] = Pothos::Object(this->remoteID);

    auto reply = env->transact(req);

    return reply["result"].extract<size_t>();
}

std::string RemoteProxyHandle::toString(void) const
{
    //create request
    Pothos::ObjectKwargs req;
    req["action"] = Pothos::Object("toString");
    req["handleID"] = Pothos::Object(this->remoteID);

    auto reply = env->transact(req);

    return reply["result"].extract<std::string>();
}

std::string RemoteProxyHandle::getClassName(void) const
{
    //create request
    Pothos::ObjectKwargs req;
    req["action"] = Pothos::Object("getClassName");
    req["handleID"] = Pothos::Object(this->remoteID);

    auto reply = env->transact(req);

    return reply["result"].extract<std::string>();
}
