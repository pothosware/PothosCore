// Copyright (c) 2013-2016 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Managed/Class.hpp>
#include <Pothos/Managed/Exception.hpp>
#include <Pothos/Util/TypeInfo.hpp>
#include <Pothos/Callable.hpp>
#include <Pothos/Plugin.hpp>
#include <Poco/SingletonHolder.h>
#include <Poco/RWLock.h>
#include <Poco/Logger.h>
#include <map>

/***********************************************************************
 * Global map structure for registry
 **********************************************************************/
static Poco::RWLock &getMapMutex(void)
{
    static Poco::SingletonHolder<Poco::RWLock> sh;
    return *sh.get();
}

//singleton global map for all supported classes
typedef std::map<size_t, Pothos::Plugin> ClassMapType;
static ClassMapType &getClassMap(void)
{
    static Poco::SingletonHolder<ClassMapType> sh;
    return *sh.get();
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
        const auto &reg = plugin.getObject().extract<Pothos::ManagedClass>();

        Poco::RWLock::ScopedWriteLock lock(getMapMutex());
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
    Poco::RWLock::ScopedReadLock lock(getMapMutex());
    auto it = getClassMap().find(type.hash_code());

    //thow an error when the entry is not found
    if (it == getClassMap().end()) throw ManagedClassLookupError(
        "Pothos::ManagedClass::lookup("+Util::typeInfoToString(type)+")",
        "no registration found");

    //extract the managed class
    return it->second.getObject().extract<Pothos::ManagedClass>();
}
