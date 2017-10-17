// Copyright (c) 2013-2017 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Managed/Class.hpp>
#include <Pothos/Managed/Exception.hpp>
#include <Pothos/Util/SpinLockRW.hpp>
#include <Pothos/Util/TypeInfo.hpp>
#include <Pothos/Callable.hpp>
#include <Pothos/Plugin.hpp>
#include <Poco/Logger.h>
#include <mutex>
#include <map>

/***********************************************************************
 * Global map structure for registry
 **********************************************************************/
static Pothos::Util::SpinLockRW &getMapMutex(void)
{
    static Pothos::Util::SpinLockRW lock;
    return lock;
}

//singleton global map for all supported classes
typedef std::map<size_t, Pothos::Plugin> ClassMapType;
static ClassMapType &getClassMap(void)
{
    static ClassMapType map;
    return map;
}

/***********************************************************************
 * Conversion registration handling
 **********************************************************************/
static void handlePluginEvent(const Pothos::Plugin &plugin, const std::string &event)
{
    poco_debug_f2(Poco::Logger::get("Pothos.ManagedClass.handlePluginEvent"), "plugin %s, event %s", plugin.toString(), event);
    POTHOS_EXCEPTION_TRY
    {
        //validate the plugin -- if we want to handle it -- check the signature:
        if (plugin.getObject().type() != typeid(Pothos::ManagedClass)) return;
        const Pothos::ManagedClass &reg = plugin.getObject();

        std::lock_guard<Pothos::Util::SpinLockRW> lock(getMapMutex());
        if (event == "add")
        {
            getClassMap()[reg.type().hash_code()] = plugin;
            getClassMap()[reg.pointerType().hash_code()] = plugin;
            getClassMap()[reg.sharedType().hash_code()] = plugin;
        }
        if (event == "remove")
        {
            getClassMap()[reg.type().hash_code()] = Pothos::Plugin();
            getClassMap()[reg.pointerType().hash_code()] = Pothos::Plugin();
            getClassMap()[reg.sharedType().hash_code()] = Pothos::Plugin();
        }
    }
    POTHOS_EXCEPTION_CATCH(const Pothos::Exception &ex)
    {
        poco_error_f3(Poco::Logger::get("Pothos.ManagedClass.handlePluginEvent"),
            "exception %s, plugin %s, event %s", ex.displayText(), plugin.toString(), event);
    }
}

/***********************************************************************
 * Register event handler
 **********************************************************************/
pothos_static_block(pothosManagedClassRegister)
{
    Pothos::PluginRegistry::addCall("/managed", &handlePluginEvent);
}

/***********************************************************************
 * The lookup implementation
 **********************************************************************/
Pothos::ManagedClass Pothos::ManagedClass::lookup(const std::type_info &type)
{
    //find the plugin in the map, it will be null if not found
    Pothos::Util::SpinLockRW::SharedLock lock(getMapMutex());
    auto it = getClassMap().find(type.hash_code());

    //thow an error when the entry is not found
    if (it == getClassMap().end()) throw ManagedClassLookupError(
        "Pothos::ManagedClass::lookup("+Util::typeInfoToString(type)+")",
        "no registration found");

    //extract the managed class
    return it->second.getObject().extract<Pothos::ManagedClass>();
}
