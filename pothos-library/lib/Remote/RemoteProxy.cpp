// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "Remote/RemoteProxy.hpp"
#include <Pothos/Remote/Client.hpp>
#include <Pothos/Plugin.hpp>
#include <Poco/Logger.h>
#include <iostream>

Pothos::ObjectKwargs RemoteProxyEnvironment::transact(const Pothos::ObjectKwargs &reqArgs)
{
    std::lock_guard<std::mutex> lock(mutex);

    //request object
    Pothos::Object request(reqArgs);
    request.serialize(os);
    os.flush();

    //reply object
    Pothos::Object reply;
    reply.deserialize(is);
    return reply.extract<Pothos::ObjectKwargs>();
}

RemoteProxyEnvironment::RemoteProxyEnvironment(
    std::istream &is, std::ostream &os,
    const std::string &name, const Pothos::ProxyEnvironmentArgs &args
):
    is(is), os(os), name(name)
{
    //create request
    Pothos::ObjectKwargs req;
    for (const auto &entry : args)
    {
        req[entry.first] = Pothos::Object(entry.second);
    }
    req["action"] = Pothos::Object("RemoteProxyEnvironment");
    req["name"] = Pothos::Object(name);

    auto reply = this->transact(req);

    //check for an error
    auto errorMsgIt = reply.find("errorMsg");
    if (errorMsgIt != reply.end()) throw Pothos::ProxyEnvironmentFactoryError(
        "RemoteProxyEnvironment()", errorMsgIt->second.extract<std::string>());

    //set the remote ID for this env
    remoteID = reply["envID"].convert<size_t>();
    upid = reply["upid"].convert<std::string>();
}

RemoteProxyEnvironment::~RemoteProxyEnvironment(void)
{
    //create request
    Pothos::ObjectKwargs req;
    req["action"] = Pothos::Object("~RemoteProxyEnvironment");
    req["envID"] = Pothos::Object(this->remoteID);

    try
    {
        this->transact(req);
    }
    catch(const Pothos::Exception &ex)
    {
        poco_error(Poco::Logger::get("Pothos.RemoteProxyEnvironment"), "destructor threw: "+ex.displayText());
    }
}

Pothos::Proxy RemoteProxyEnvironment::makeHandle(const size_t remoteID)
{
    auto env = std::dynamic_pointer_cast<RemoteProxyEnvironment>(this->shared_from_this());
    return Pothos::Proxy(new RemoteProxyHandle(env, remoteID));
}

std::shared_ptr<RemoteProxyHandle> RemoteProxyEnvironment::getHandle(const Pothos::Proxy &proxy)
{
    //check if the proxy environment is for the same server
    auto remoteEnv = std::dynamic_pointer_cast<RemoteProxyEnvironment>(proxy.getEnvironment());
    if (remoteEnv and this->upid == remoteEnv->upid)
    {
        return std::dynamic_pointer_cast<RemoteProxyHandle>(proxy.getHandle());
    }

    //otherwise perform the conversion
    auto local = proxy.getEnvironment()->convertProxyToObject(proxy);
    auto myProxy = this->convertObjectToProxy(local);
    return std::dynamic_pointer_cast<RemoteProxyHandle>(myProxy.getHandle());
}

Pothos::Proxy RemoteProxyEnvironment::findProxy(const std::string &name)
{
    //create request
    Pothos::ObjectKwargs req;
    req["action"] = Pothos::Object("findProxy");
    req["envID"] = Pothos::Object(this->remoteID);
    req["name"] = Pothos::Object(name);

    auto reply = this->transact(req);

    //check for an error
    auto errorMsgIt = reply.find("errorMsg");
    if (errorMsgIt != reply.end()) throw Pothos::ProxyEnvironmentFindError(
        "RemoteProxyEnvironment::findProxy("+name+")", errorMsgIt->second.extract<std::string>());

    //otherwise make a handle
    return this->makeHandle(reply["handleID"].convert<size_t>());
}

Pothos::Proxy RemoteProxyEnvironment::convertObjectToProxy(const Pothos::Object &local)
{
    //create request
    Pothos::ObjectKwargs req;
    req["action"] = Pothos::Object("convertObjectToProxy");
    req["envID"] = Pothos::Object(this->remoteID);
    req["local"] = local;

    auto reply = this->transact(req);

    //check for an error
    auto errorMsgIt = reply.find("errorMsg");
    if (errorMsgIt != reply.end()) throw Pothos::ProxyEnvironmentConvertError(
        "RemoteProxyEnvironment::convertObjectToProxy()", errorMsgIt->second.extract<std::string>());

    //otherwise make a handle
    return this->makeHandle(reply["handleID"].convert<size_t>());
}

Pothos::Object RemoteProxyEnvironment::convertProxyToObject(const Pothos::Proxy &proxy)
{
    auto handle = this->getHandle(proxy);

    //create request
    Pothos::ObjectKwargs req;
    req["action"] = Pothos::Object("convertProxyToObject");
    req["envID"] = Pothos::Object(this->remoteID);
    req["handleID"] = Pothos::Object(handle->remoteID);

    auto reply = this->transact(req);

    //check for an error
    auto errorMsgIt = reply.find("errorMsg");
    if (errorMsgIt != reply.end()) throw Pothos::ProxyEnvironmentConvertError(
        "RemoteProxyEnvironment::convertProxyToObject()", errorMsgIt->second.extract<std::string>());

    return reply["local"];
}

/***********************************************************************
 * factory method
 **********************************************************************/
Pothos::ProxyEnvironment::Sptr Pothos::RemoteClient::makeEnvironment(
    std::istream &is, std::ostream &os,
    const std::string &name,
    const ProxyEnvironmentArgs &args)
{
    return Pothos::ProxyEnvironment::Sptr(new RemoteProxyEnvironment(is, os, name, args));
}

Pothos::ProxyEnvironment::Sptr Pothos::RemoteClient::makeEnvironment(
    std::iostream &io,
    const std::string &name,
    const ProxyEnvironmentArgs &args)
{
    return makeEnvironment(io, io, name, args);
}
