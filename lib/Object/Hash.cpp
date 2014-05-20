// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Object/Object.hpp>
#include <Pothos/Object/Exception.hpp>
#include <Pothos/Callable.hpp>
#include <Pothos/Plugin.hpp>
#include <Poco/SingletonHolder.h>
#include <Poco/RWLock.h>
#include <Poco/Logger.h>
#include <Poco/Format.h>
#include <map>

/***********************************************************************
 * Global map structure for comparisons
 **********************************************************************/
static Poco::RWLock &getMapMutex(void)
{
    static Poco::SingletonHolder<Poco::RWLock> sh;
    return *sh.get();
}

//singleton global map for all supported comparisons
typedef std::map<size_t, Pothos::Plugin> HashFcnMapType;
static HashFcnMapType &getHashFcnMap(void)
{
    static Poco::SingletonHolder<HashFcnMapType> sh;
    return *sh.get();
}

/***********************************************************************
 * Comparison registration handling
 **********************************************************************/
static void handleHashFcnPluginEvent(const Pothos::Plugin &plugin, const std::string &event)
{
    poco_information_f2(Poco::Logger::get("Pothos.Object.handleHashFcnPluginEvent"), "plugin %s, event %s", plugin.toString(), event);
    try
    {
        //validate the plugin -- if we want to handle it -- check the signature:
        if (plugin.getObject().type() != typeid(Pothos::Callable)) return;
        const auto &call = plugin.getObject().extract<Pothos::Callable>();
        if (call.type(-1) != typeid(size_t)) return;
        if (call.getNumArgs() != 1) return;

        Poco::RWLock::ScopedWriteLock lock(getMapMutex());
        if (event == "add")
        {
            getHashFcnMap()[call.type(0).hash_code()] = plugin;
        }
        if (event == "remove")
        {
            getHashFcnMap()[call.type(0).hash_code()] = Pothos::Plugin();
        }
    }
    catch(const Pothos::Exception &ex)
    {
        poco_error_f3(Poco::Logger::get("Pothos.Object.handleHashFcnPluginEvent"),
            "exception %s, plugin %s, event %s", ex.displayText(), plugin.toString(), event);
    }
    catch(...)
    {
        poco_error_f3(Poco::Logger::get("Pothos.Object.handleHashFcnPluginEvent"),
            "exception %s, plugin %s, event %s", std::string("unknown"), plugin.toString(), event);
    }
}

/***********************************************************************
 * Register event handler
 **********************************************************************/
pothos_static_block(pothosObjectHashFcnRegister)
{
    Pothos::PluginRegistry::addCall("/object/hash", &handleHashFcnPluginEvent);
}

/***********************************************************************
 * The hash code implementation
 **********************************************************************/
size_t Pothos::Object::hashCode(void) const
{
    //find the plugin in the map, it will be null if not found
    Poco::RWLock::ScopedReadLock lock(getMapMutex());
    auto it = getHashFcnMap().find(this->type().hash_code());

    //return the address when no hash function found
    if (it == getHashFcnMap().end()) return size_t(_impl);

    auto call = it->second.getObject().extract<Pothos::Callable>();
    return call.opaqueCall(this, 1).extract<size_t>();
}
