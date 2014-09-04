// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "Remote/RemoteProxy.hpp"
#include <Pothos/Remote/Client.hpp>
#include <Pothos/Plugin.hpp>
#include <Poco/Logger.h>
#include <iostream>
#include <sstream>
#include <thread>

Pothos::ObjectKwargs RemoteProxyEnvironment::transact(const Pothos::ObjectKwargs &reqArgs_)
{
    if (not connectionActive)
    {
        throw Pothos::IOException("RemoteProxyEnvironment::transact()", "connection inactive");
    }

    //add the thread ID to the args
    const auto tid = std::hash<std::thread::id>()(std::this_thread::get_id());
    auto reqArgs = reqArgs_;
    reqArgs["tid"] = Pothos::Object(tid);

    try
    {
        //send request object over output stream
        {
            std::lock_guard<std::mutex> lock(osMutex);
            Pothos::Object request(reqArgs);
            request.serialize(os);
            os.flush();
        }

        //wait for reply object
        while (true)
        {
            std::lock_guard<std::mutex> lock(isMutex);

            //is there a reply in the cache?
            auto it = tidToReply.find(tid);
            if (it != tidToReply.end())
            {
                auto reply = it->second;
                tidToReply.erase(it);
                return reply;
            }

            //otherwise wait on input stream
            Pothos::Object reply;
            reply.deserialize(is);
            const auto replyArgs = reply.extract<Pothos::ObjectKwargs>();
            const auto replyTid = replyArgs.at("tid").convert<size_t>();
            if (replyTid == tid) return replyArgs;
            tidToReply[replyTid] = replyArgs;
        }
    }
    catch (const Poco::IOException &ex)
    {
        connectionActive = false;
        throw Pothos::IOException("RemoteProxyEnvironment::transact()", ex.message());
    }
}

RemoteProxyEnvironment::RemoteProxyEnvironment(
    std::istream &is, std::ostream &os,
    const std::string &name, const Pothos::ProxyEnvironmentArgs &args
):
    is(is), os(os), name(name), connectionActive(true)
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
    nodeId = reply["nodeId"].convert<std::string>();
    peerAddr = reply["peerAddr"].convert<std::string>();
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
