// Copyright (c) 2013-2017 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Object/Object.hpp>
#include <Pothos/Object/Exception.hpp>
#include <Pothos/Util/SpinLockRW.hpp>
#include <Pothos/Callable.hpp>
#include <Pothos/Plugin.hpp>
#include <Poco/Logger.h>
#include <Poco/Format.h>
#include <mutex>
#include <map>

/***********************************************************************
 * Global map structure for comparisons
 **********************************************************************/
static Pothos::Util::SpinLockRW &getMapMutex(void)
{
    static Pothos::Util::SpinLockRW lock;
    return lock;
}

//singleton global map for all supported comparisons
typedef std::map<size_t, Pothos::Plugin> HashFcnMapType;
static HashFcnMapType &getHashFcnMap(void)
{
    static HashFcnMapType map;
    return map;
}

/***********************************************************************
 * Comparison registration handling
 **********************************************************************/
static void handleHashFcnPluginEvent(const Pothos::Plugin &plugin, const std::string &event)
{
    poco_debug_f2(Poco::Logger::get("Pothos.Object.handleHashFcnPluginEvent"), "plugin %s, event %s", plugin.toString(), event);
    POTHOS_EXCEPTION_TRY
    {
        //validate the plugin -- if we want to handle it -- check the signature:
        if (plugin.getObject().type() != typeid(Pothos::Callable)) return;
        const auto &call = plugin.getObject().extract<Pothos::Callable>();
        if (call.type(-1) != typeid(size_t)) return;
        if (call.getNumArgs() != 1) return;

        std::lock_guard<Pothos::Util::SpinLockRW> lock(getMapMutex());
        if (event == "add")
        {
            getHashFcnMap()[call.type(0).hash_code()] = plugin;
        }
        if (event == "remove")
        {
            getHashFcnMap()[call.type(0).hash_code()] = Pothos::Plugin();
        }
    }
    POTHOS_EXCEPTION_CATCH(const Pothos::Exception &ex)
    {
        poco_error_f3(Poco::Logger::get("Pothos.Object.handleHashFcnPluginEvent"),
            "exception %s, plugin %s, event %s", ex.displayText(), plugin.toString(), event);
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
    Pothos::Util::SpinLockRW::SharedLock lock(getMapMutex());
    auto it = getHashFcnMap().find(this->type().hash_code());

    //return the address when no hash function found
    if (it == getHashFcnMap().end()) return size_t(_impl);

    auto call = it->second.getObject().extract<Pothos::Callable>();
    return call.opaqueCall(this, 1).extract<size_t>();
}
