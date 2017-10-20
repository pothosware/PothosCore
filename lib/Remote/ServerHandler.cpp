// Copyright (c) 2013-2017 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "RemoteProxyDatagram.hpp"
#include <Pothos/Object.hpp>
#include <Pothos/Object/Containers.hpp>
#include <Pothos/Proxy.hpp>
#include <Pothos/Remote/Handler.hpp>
#include <Pothos/System/HostInfo.hpp>
#include <Poco/Exception.h>
#include <Poco/Bugcheck.h>
#include <iostream>
#include <mutex>
#include <map>

/***********************************************************************
 * Active objects on the server
 **********************************************************************/
static std::mutex &getObjectsMutex(void)
{
    static std::mutex mutex;
    return mutex;
}

typedef std::map<size_t, Pothos::Object> ServerObjectsMapType;
static ServerObjectsMapType &getObjectsMap(void)
{
    static ServerObjectsMapType map;
    return map;
}

static Pothos::Object getNewObjectId(const Pothos::Object &obj)
{
    std::lock_guard<std::mutex> lock(getObjectsMutex());
    static size_t id = 0;
    getObjectsMap()[++id] = obj;
    return Pothos::Object(id);
}

static Pothos::Object getObjectAtId(const Pothos::Object &id)
{
    const size_t key(id);
    std::lock_guard<std::mutex> lock(getObjectsMutex());
    return getObjectsMap()[key];
}

static void removeObjectAtId(const Pothos::Object &id)
{
    const size_t key(id);
    std::lock_guard<std::mutex> lock(getObjectsMutex());
    getObjectsMap().erase(key);
}

/***********************************************************************
 * Handler implementation
 **********************************************************************/
bool Pothos::RemoteHandler::runHandlerOnce(std::istream &is, std::ostream &os)
{
    bool done = false;

    //deserialize the request
    const auto reqArgs = recvDatagram(is);

    //process the request and form the reply
    Pothos::ObjectKwargs replyArgs;
    replyArgs["tid"] = reqArgs.at("tid");
    POTHOS_EXCEPTION_TRY
    {
        const std::string &action = reqArgs.at("action");
        if (action == "RemoteProxyEnvironment")
        {
            Pothos::ProxyEnvironmentArgs envArgs;
            for (const auto &entry : reqArgs)
            {
                if (entry.second.type() != typeid(std::string)) continue;
                envArgs[entry.first] = entry.second.extract<std::string>();
            }
            const std::string &name = reqArgs.at("name");
            auto env = Pothos::ProxyEnvironment::make(name, envArgs);
            replyArgs["envID"] = getNewObjectId(Pothos::Object(env));

            //a unique process ID for this server
            const auto info = Pothos::System::HostInfo::get();
            replyArgs["upid"] = Pothos::Object(Pothos::ProxyEnvironment::getLocalUniquePid());
            replyArgs["nodeId"] = Pothos::Object(info.nodeId);
            replyArgs["peerAddr"] = Pothos::Object(_peerAddr);
        }
        else if (action == "~RemoteProxyEnvironment")
        {
            removeObjectAtId(reqArgs.at("envID"));
            done = true;
        }
        else if (action == "findProxy")
        {
            const Pothos::ProxyEnvironment::Sptr &env = getObjectAtId(reqArgs.at("envID"));
            auto proxy = env->findProxy(reqArgs.at("name").extract<std::string>());
            replyArgs["handleID"] = getNewObjectId(Pothos::Object(proxy));
        }
        else if (action == "convertObjectToProxy")
        {
            const Pothos::ProxyEnvironment::Sptr &env = getObjectAtId(reqArgs.at("envID"));
            auto proxy = env->convertObjectToProxy(reqArgs.at("local"));
            replyArgs["handleID"] = getNewObjectId(Pothos::Object(proxy));
        }
        else if (action == "convertProxyToObject")
        {
            const Pothos::ProxyEnvironment::Sptr &env = getObjectAtId(reqArgs.at("envID"));
            const Pothos::Proxy &proxy = getObjectAtId(reqArgs.at("handleID"));
            auto local = env->convertProxyToObject(proxy);
            replyArgs["local"] = local;
        }
        else if (action == "~RemoteProxyHandle")
        {
            removeObjectAtId(reqArgs.at("handleID"));
        }
        else if (action == "call")
        {
            const Pothos::Proxy &proxy = getObjectAtId(reqArgs.at("handleID"));

            //load the args
            std::vector<Pothos::Proxy> args;
            size_t argNo = 0;
            while (true)
            {
                auto it = reqArgs.find(std::to_string(argNo++));
                if (it == reqArgs.end()) break;
                args.push_back(getObjectAtId(it->second).extract<Pothos::Proxy>());
            }

            //make the call
            try
            {
                const std::string &name = reqArgs.at("name");
                auto result = proxy.getHandle()->call(name, args.data(), args.size());
                replyArgs["handleID"] = getNewObjectId(Pothos::Object(result));
            }
            catch (const Pothos::ProxyExceptionMessage &ex)
            {
                replyArgs["message"] = Pothos::Object(ex.message());
            }
        }
        else if (action == "compareTo")
        {
            const Pothos::Proxy &proxy = getObjectAtId(reqArgs.at("handleID"));
            const Pothos::Proxy &other = getObjectAtId(reqArgs.at("otherID"));
            replyArgs["result"] = Pothos::Object(proxy.compareTo(other));
        }
        else if (action == "hashCode")
        {
            const Pothos::Proxy &proxy = getObjectAtId(reqArgs.at("handleID"));
            replyArgs["result"] = Pothos::Object(proxy.hashCode());
        }
        else if (action == "toString")
        {
            const Pothos::Proxy &proxy = getObjectAtId(reqArgs.at("handleID"));
            replyArgs["result"] = Pothos::Object(proxy.toString());
        }
        else if (action == "getClassName")
        {
            const Pothos::Proxy &proxy = getObjectAtId(reqArgs.at("handleID"));
            replyArgs["result"] = Pothos::Object(proxy.getClassName());
        }
        else
        {
            poco_bugcheck_msg(action.c_str());
        }
    }
    POTHOS_EXCEPTION_CATCH(const Pothos::Exception &ex)
    {
        replyArgs["errorMsg"] = Pothos::Object(ex.displayText());
    }

    //serialize the reply
    sendDatagram(os, replyArgs);

    return done;
}

Pothos::RemoteHandler::RemoteHandler(void)
{
    return;
}

Pothos::RemoteHandler::RemoteHandler(const std::string &peerAddr):
    _peerAddr(peerAddr)
{
    return;
}

void Pothos::RemoteHandler::runHandler(std::istream &is, std::ostream &os)
{
    bool done = false;
    while (is.good() and os.good() and not done)
    {
        done = runHandlerOnce(is, os);
    }
}

void Pothos::RemoteHandler::runHandler(std::iostream &io)
{
    return runHandler(io, io);
}

#include <Pothos/Managed.hpp>

static auto managedRemoteHandler = Pothos::ManagedClass()
    .registerConstructor<Pothos::RemoteHandler>()
    .registerConstructor<Pothos::RemoteHandler, std::string>()
    .registerMethod<void, Pothos::RemoteHandler, std::iostream &>(POTHOS_FCN_TUPLE(Pothos::RemoteHandler, runHandler))
    .registerMethod<void, Pothos::RemoteHandler, std::istream &, std::ostream &>(POTHOS_FCN_TUPLE(Pothos::RemoteHandler, runHandler))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::RemoteHandler, runHandlerOnce))
    .commit("Pothos/RemoteHandler");
