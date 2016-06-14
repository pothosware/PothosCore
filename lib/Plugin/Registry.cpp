// Copyright (c) 2013-2016 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Plugin/Registry.hpp>
#include <Pothos/Plugin/Exception.hpp>
#include <Pothos/Callable.hpp> //gets call implementation
#include <Poco/RWLock.h>
#include <Poco/Logger.h>
#include <Poco/SingletonHolder.h>
#include <cassert>
#include <map>

/***********************************************************************
 * registry data structure
 **********************************************************************/
static Poco::RWLock &getRegistryMutex(void)
{
    static Poco::SingletonHolder<Poco::RWLock> sh;
    return *sh.get();
}

struct RegistryEntry
{
    RegistryEntry(void):
        hasPlugin(false){}
    Pothos::Plugin plugin;
    bool hasPlugin;
    std::vector<std::string> nodeNamesOrdered; //so we know the order that they were added
    std::map<std::string, RegistryEntry> nodes;

    //count number of plugins here and deeper
    size_t getNumPlugins(void) const
    {
        size_t count = 0;
        if (this->hasPlugin) count++;
        for (const auto &entry : this->nodes)
        {
            count += entry.second.getNumPlugins();
        }
        return count;
    }
};

static RegistryEntry &getRegistryRoot(void)
{
    static Poco::SingletonHolder<RegistryEntry> sh;
    return *sh.get();
}

/***********************************************************************
 * plugin event handler
 **********************************************************************/
static bool canObjectHandleEvent(const Pothos::Object &obj)
{
    if (obj.type() != typeid(Pothos::Callable)) return false; //its not a call
    const Pothos::Callable &call = obj.extract<Pothos::Callable>();
    if (not call) return false; //its null
    //check the signature
    if (call.type(-1) != typeid(void)) return false;
    if (call.getNumArgs() != 2) return false;
    if (call.type(0) != typeid(Pothos::Plugin)) return false;
    if (call.type(1) != typeid(std::string)) return false;
    return true;
}

static void callPluginEventHandler(const Pothos::Object &handler, const Pothos::Plugin &plugin, const std::string &event)
{
    if (not canObjectHandleEvent(handler)) return;
    POTHOS_EXCEPTION_TRY
    {
        handler.extract<Pothos::Callable>().callVoid(plugin, event);
    }
    POTHOS_EXCEPTION_CATCH(const Pothos::Exception &ex)
    {
        poco_error_f3(Poco::Logger::get("Pothos.PluginRegistry.handlePluginEvent"),
        "exception %s, plugin %s, event %s", ex.displayText(), plugin.toString(), event);
    }
}

static void handlePluginEvent(const Pothos::Plugin &plugin, const std::string &event)
{
    const Pothos::PluginPath &path = plugin.getPath();
    std::vector<Pothos::Plugin> parentPlugins;

    //traverse the tree - store a list of parent plugins
    //we lock the mutex here for protection and make a plugin copy
    {
        Poco::RWLock::ScopedReadLock lock(getRegistryMutex());
        const std::vector<std::string> pathNodes = path.listNodes();
        RegistryEntry *root = &getRegistryRoot();

        for (size_t i = 0; i+1 < pathNodes.size(); i++)
        {
            parentPlugins.insert(parentPlugins.begin(), root->plugin);
            //next node in the tree at this node name
            root = &root->nodes[pathNodes[i]];
        }
        parentPlugins.insert(parentPlugins.begin(), root->plugin);
    }

    //traverse back up the plugin tree -- calling all potential handlers
    for (size_t i = 0; i < parentPlugins.size(); i++)
    {
        callPluginEventHandler(parentPlugins[i].getObject(), plugin, event);
    }
}

//if the plugin is an event handler, and it just got added,
//then what we do is do the event add on all sub-tree plugins
static void handleMissedSubTreeEvents(const Pothos::Object &handler, const Pothos::PluginPath &path)
{
    for (const auto &subdir : Pothos::PluginRegistry::list(path))
    {
        auto subPath = path.join(subdir);
        try
        {
            callPluginEventHandler(handler, Pothos::PluginRegistry::get(subPath), "add");
        }
        catch(const Pothos::PluginRegistryError &)
        {
        }
        handleMissedSubTreeEvents(handler, subPath);
    }
}

/***********************************************************************
 * active module implementation
 **********************************************************************/
static Pothos::PluginModule &getActiveModuleLoading(void)
{
    static Poco::SingletonHolder<Pothos::PluginModule> sh;
    return *sh.get();
}

void registrySetActiveModuleLoading(const Pothos::PluginModule &module)
{
    //made thread safe by lock in caller routine from Module.cpp
    getActiveModuleLoading() = module;
}

void updatePluginAssociation(const std::string &action, const Pothos::Plugin &plugin);

/***********************************************************************
 * Registry implementation
 **********************************************************************/
void Pothos::PluginRegistry::add(const Plugin &plugin_)
{
    //copy the plugin -- we need an rw copy to attach module to
    Plugin plugin = plugin_;
    const PluginPath &path = plugin.getPath();

    poco_debug(Poco::Logger::get("Pothos.PluginRegistry.add"), plugin.toString());

    {
        Poco::RWLock::ScopedWriteLock lock(getRegistryMutex());
        const std::vector<std::string> pathNodes = path.listNodes();
        RegistryEntry *root = &getRegistryRoot();

        for (size_t i = 0; i < pathNodes.size(); i++)
        {
            auto it = std::find(root->nodeNamesOrdered.begin(), root->nodeNamesOrdered.end(), pathNodes[i]);
            if (it == root->nodeNamesOrdered.end()) root->nodeNamesOrdered.push_back(pathNodes[i]);

            //next node in the tree at this node name
            root = &root->nodes[pathNodes[i]];
        }

        //throw if the root already has a plugin
        if (root->hasPlugin)
        {
            const auto newModulePath = getActiveModuleLoading().getFilePath();
            const auto currentModulePath = root->plugin.getModule().getFilePath();
            throw Pothos::PluginRegistryError("Pothos::PluginRegistry::add("+path.toString()+")", Poco::format(
                "plugin already registered\n\tLoading: %s, Conflicts: %s", newModulePath, currentModulePath));
        }

        //store the plugin and attach the module
        plugin = Plugin(plugin.getPath(), plugin.getObject(), getActiveModuleLoading());
        updatePluginAssociation("add", plugin);
        root->hasPlugin = true;
        root->plugin = plugin;
    }

    handlePluginEvent(plugin, "add");
    handleMissedSubTreeEvents(plugin.getObject(), plugin.getPath());
}

Pothos::Plugin Pothos::PluginRegistry::get(const PluginPath &path)
{
    Poco::RWLock::ScopedReadLock lock(getRegistryMutex());
    const std::vector<std::string> pathNodes = path.listNodes();
    RegistryEntry *root = &getRegistryRoot();

    for (size_t i = 0; i < pathNodes.size(); i++)
    {
        //next node in the tree at this node name
        root = &root->nodes[pathNodes[i]];
    }

    //throw if the root does not have a plugin
    if (not root->hasPlugin)
    {
        throw Pothos::PluginRegistryError("Pothos::PluginRegistry::get("+path.toString()+")", "plugin path not found");
    }

    return root->plugin;
}

Pothos::Plugin Pothos::PluginRegistry::remove(const PluginPath &path)
{
    poco_debug(Poco::Logger::get("Pothos.PluginRegistry.remove"), path.toString());

    Plugin plugin;
    {
        Poco::RWLock::ScopedWriteLock lock(getRegistryMutex());
        const std::vector<std::string> pathNodes = path.listNodes();
        RegistryEntry *root = &getRegistryRoot();

        for (size_t i = 0; i < pathNodes.size(); i++)
        {
            //next node in the tree at this node name
            root = &root->nodes[pathNodes[i]];
        }

        //throw if the root does not have a plugin
        if (not root->hasPlugin)
        {
            throw Pothos::PluginRegistryError("Pothos::PluginRegistry::remove("+path.toString()+")", "plugin path not found");
        }

        //clear plugin entry and return result
        plugin = root->plugin;
        updatePluginAssociation("remove", plugin);
        root->hasPlugin = false;
        root->plugin = Plugin(); //clears
    }

    handlePluginEvent(plugin, "remove");
    return plugin;
}

bool Pothos::PluginRegistry::empty(const PluginPath &path)
{
    Poco::RWLock::ScopedReadLock lock(getRegistryMutex());
    const std::vector<std::string> pathNodes = path.listNodes();
    RegistryEntry *root = &getRegistryRoot();

    for (size_t i = 0; i < pathNodes.size(); i++)
    {
        //next node in the tree at this node name
        root = &root->nodes[pathNodes[i]];
    }

    return not root->hasPlugin;
}

bool Pothos::PluginRegistry::exists(const PluginPath &path)
{
    Poco::RWLock::ScopedReadLock lock(getRegistryMutex());
    const std::vector<std::string> pathNodes = path.listNodes();
    RegistryEntry *root = &getRegistryRoot();

    for (size_t i = 0; i < pathNodes.size(); i++)
    {
        //next node in the tree at this node name
        root = &root->nodes[pathNodes[i]];
    }

    return root->getNumPlugins() != 0;
}

std::vector<std::string> Pothos::PluginRegistry::list(const PluginPath &path)
{
    Poco::RWLock::ScopedReadLock lock(getRegistryMutex());
    const std::vector<std::string> pathNodes = path.listNodes();
    RegistryEntry *root = &getRegistryRoot();

    for (size_t i = 0; i < pathNodes.size(); i++)
    {
        //next node in the tree at this node name
        root = &root->nodes[pathNodes[i]];
    }

    std::vector<std::string> nodes;
    for (const auto &name : root->nodeNamesOrdered)
    {
        if (root->nodes[name].getNumPlugins() != 0) nodes.push_back(name);
    }
    return nodes;
}

Pothos::PluginRegistryInfoDump::PluginRegistryInfoDump(void)
{
    return;
}

static void loadInfoDump(const Pothos::PluginPath &path, const RegistryEntry &entry, Pothos::PluginRegistryInfoDump &dump)
{
    dump.pluginPath = path.toString();
    if (entry.hasPlugin)
    {
        assert(path == entry.plugin.getPath());
        const auto &obj = entry.plugin.getObject();
        if (obj) dump.objectType = obj.toString();
        dump.modulePath = entry.plugin.getModule().getFilePath();
    }
    for (const auto &name : entry.nodeNamesOrdered)
    {
        dump.subInfo.push_back(Pothos::PluginRegistryInfoDump());
        loadInfoDump(path.join(name), entry.nodes.at(name), dump.subInfo.back());
    }
}

Pothos::PluginRegistryInfoDump Pothos::PluginRegistry::dump(void)
{
    Poco::RWLock::ScopedReadLock lock(getRegistryMutex());
    PluginRegistryInfoDump dump;
    loadInfoDump(PluginPath(), getRegistryRoot(), dump);
    return dump;
}

#include <Pothos/Managed.hpp>

static auto managedPluginRegistry = Pothos::ManagedClass()
    .registerClass<Pothos::PluginRegistry>()
    .registerStaticMethod<const Pothos::Plugin &, void>("add", &Pothos::PluginRegistry::add)
    .registerStaticMethod(POTHOS_FCN_TUPLE(Pothos::PluginRegistry, remove))
    .registerStaticMethod(POTHOS_FCN_TUPLE(Pothos::PluginRegistry, get))
    .registerStaticMethod(POTHOS_FCN_TUPLE(Pothos::PluginRegistry, empty))
    .registerStaticMethod(POTHOS_FCN_TUPLE(Pothos::PluginRegistry, exists))
    .registerStaticMethod(POTHOS_FCN_TUPLE(Pothos::PluginRegistry, list))
    .registerStaticMethod(POTHOS_FCN_TUPLE(Pothos::PluginRegistry, dump))
    .commit("Pothos/PluginRegistry");


#include <Pothos/Object/Serialize.hpp>

namespace Pothos { namespace serialization {
template <class Archive>
void serialize(Archive &ar, Pothos::PluginRegistryInfoDump &t, const unsigned int)
{
    ar & t.pluginPath;
    ar & t.objectType;
    ar & t.modulePath;
    ar & t.subInfo;
}
}}

POTHOS_OBJECT_SERIALIZE(Pothos::PluginRegistryInfoDump)
POTHOS_OBJECT_SERIALIZE(std::vector<Pothos::PluginRegistryInfoDump>)
