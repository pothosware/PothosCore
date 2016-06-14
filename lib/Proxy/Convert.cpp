// Copyright (c) 2013-2016 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Proxy/Environment.hpp>
#include <Pothos/Proxy/Handle.hpp>
#include <Pothos/Proxy/Exception.hpp>
#include <Pothos/Util/TypeInfo.hpp>
#include <Pothos/Callable.hpp>
#include <Pothos/Plugin.hpp>
#include <Poco/SingletonHolder.h>
#include <Poco/RWLock.h>
#include <Poco/Logger.h>
#include <Poco/Hash.h>
#include <iostream>
#include <cassert>
#include <map>

/***********************************************************************
 * Global map structure for conversions
 **********************************************************************/
static Poco::RWLock &getMapToLocalMutex(void)
{
    static Poco::SingletonHolder<Poco::RWLock> sh;
    return *sh.get();
}

static Poco::RWLock &getMapToProxyMutex(void)
{
    static Poco::SingletonHolder<Poco::RWLock> sh;
    return *sh.get();
}

typedef std::map<size_t, Pothos::Plugin> ConvertMapType;

static ConvertMapType &getConvertToLocalMap(void)
{
    static Poco::SingletonHolder<ConvertMapType> sh;
    return *sh.get();
}

static ConvertMapType &getConvertToProxyMap(void)
{
    static Poco::SingletonHolder<ConvertMapType> sh;
    return *sh.get();
}

/***********************************************************************
 * hash functions
 **********************************************************************/
static size_t hashIt(const std::string &name, const std::type_info &type)
{
    return type.hash_code() ^ Poco::hash(name);
}

static size_t hashIt(const std::string &name, const std::string &type)
{
    return Poco::hash(type) ^ Poco::hash(name);
}

/***********************************************************************
 * Validate registry entries
 **********************************************************************/
static bool isConvertToLocal(const Pothos::Plugin &plugin)
{
    if (plugin.getObject().type() != typeid(Pothos::ProxyConvertPair)) return false;
    const auto &pair = plugin.getObject().extract<Pothos::ProxyConvertPair>();
    const auto &callable = pair.second;
    if (callable.getNumArgs() != 1) return false;
    if (callable.type(0) != typeid(Pothos::Proxy)) return false;
    return true;
}

static bool isConvertToProxy(const Pothos::Plugin &plugin)
{
    if (plugin.getObject().type() != typeid(Pothos::Callable)) return false;
    const auto &callable = plugin.getObject().extract<Pothos::Callable>();
    if (callable.getNumArgs() != 2) return false;
    if (callable.type(-1) != typeid(Pothos::Proxy)) return false;
    if (callable.type(0) != typeid(Pothos::ProxyEnvironment::Sptr)) return false;
    return true;
}

/***********************************************************************
 * Conversion registration handling
 **********************************************************************/
static void handlePluginEvent(const Pothos::Plugin &plugin, const std::string &event)
{
    poco_debug_f2(Poco::Logger::get("Pothos.Proxy.handlePluginEvent"), "plugin %s, event %s", plugin.toString(), event);

    const auto &nodes = plugin.getPath().listNodes();
    assert(nodes.size() > 2);
    assert(nodes[0] == "proxy");
    assert(nodes[1] == "converters");
    const auto &name = nodes[2];

    POTHOS_EXCEPTION_TRY
    {
        if (isConvertToLocal(plugin))
        {
            auto pair = plugin.getObject().extract<Pothos::ProxyConvertPair>();
            Poco::RWLock::ScopedWriteLock lock(getMapToLocalMutex());
            if (event == "add")
            {
                getConvertToLocalMap()[hashIt(name, pair.first)] = plugin;
            }
            if (event == "remove")
            {
                getConvertToLocalMap()[hashIt(name, pair.first)] = Pothos::Plugin();
            }
        }
        else if (isConvertToProxy(plugin))
        {
            auto callable = plugin.getObject().extract<Pothos::Callable>();
            Poco::RWLock::ScopedWriteLock lock(getMapToProxyMutex());
            if (event == "add")
            {
                getConvertToProxyMap()[hashIt(name, callable.type(1))] = plugin;
            }
            if (event == "remove")
            {
                getConvertToProxyMap()[hashIt(name, callable.type(1))] = Pothos::Plugin();
            }
        }
        else
        {
            throw Pothos::Exception("unknown format for plugin");
        }
    }
    POTHOS_EXCEPTION_CATCH(const Pothos::Exception &ex)
    {
        poco_error_f3(Poco::Logger::get("Pothos.Proxy.handlePluginEvent"),
            "exception %s, plugin %s, event %s", ex.displayText(), plugin.toString(), event);
    }
}

/***********************************************************************
 * Register event handler
 **********************************************************************/
pothos_static_block(pothosProxyConvertRegister)
{
    Pothos::PluginRegistry::addCall("/proxy/converters", &handlePluginEvent);
}

/***********************************************************************
 * The conversion implementation
 **********************************************************************/
Pothos::Proxy Pothos::ProxyEnvironment::convertObjectToProxy(const Pothos::Object &local)
{
    //find the plugin in the map, it will be null if not found
    Poco::RWLock::ScopedReadLock lock(getMapToProxyMutex());
    const size_t h = hashIt(this->getName(), local.type());
    auto it = getConvertToProxyMap().find(h);

    //thow an error when the conversion is not supported
    if (it == getConvertToProxyMap().end()) throw Pothos::ProxyEnvironmentConvertError(
        "Pothos::ProxyEnvironment::convertObjectToProxy()",
        Poco::format("doesnt support Object of type %s to %s environment",
        local.getTypeString(), this->getName()));

    const auto &callable = it->second.getObject().extract<Pothos::Callable>();
    Pothos::Object args[2];
    args[0] = Pothos::Object(this->shared_from_this());
    args[1] = local;
    return callable.opaqueCall(args, 2).extract<Pothos::Proxy>();
}

Pothos::Object Pothos::ProxyEnvironment::convertProxyToObject(const Pothos::Proxy &proxy_)
{
    //make sure proxy is in this environment
    Pothos::Proxy proxy = proxy_;
    if (proxy.getEnvironment() != this->shared_from_this())
    {
        auto local = proxy.getEnvironment()->convertProxyToObject(proxy);
        proxy = this->convertObjectToProxy(local);
    }

    //find the plugin in the map, it will be null if not found
    Poco::RWLock::ScopedReadLock lock(getMapToLocalMutex());
    const size_t h = hashIt(this->getName(), proxy.getHandle()->getClassName());
    auto it = getConvertToLocalMap().find(h);

    //thow an error when the conversion is not supported
    if (it == getConvertToLocalMap().end()) throw Pothos::ProxyEnvironmentConvertError(
        "Pothos::ProxyEnvironment::convertProxyToObject()",
        Poco::format("doesnt support environment %s type %s to Object",
        this->getName(), std::string(proxy.getHandle()->getClassName())));

    const auto &callable = it->second.getObject().extract<Pothos::ProxyConvertPair>().second;
    Pothos::Object args[1];
    args[0] = Pothos::Object(proxy);
    return callable.opaqueCall(args, 1);
}
