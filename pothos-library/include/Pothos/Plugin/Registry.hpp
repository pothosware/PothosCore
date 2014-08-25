///
/// \file Plugin/Registry.hpp
///
/// PluginRegistry is a collection of plugins organized into a tree structure.
///
/// \copyright
/// Copyright (c) 2013-2014 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Plugin/Plugin.hpp>
#include <Pothos/Plugin/Path.hpp>
#include <string>
#include <vector>

namespace Pothos {

/*!
 * Structure for holding a dump of the registry info.
 */
struct POTHOS_API PluginRegistryInfoDump
{
    PluginRegistryInfoDump(void);
    std::string pluginPath;
    std::string objectType;
    std::string modulePath;
    std::vector<PluginRegistryInfoDump> subInfo;
};

/*!
 * PluginRegistry is a singleton class with static access methods.
 */
class POTHOS_API PluginRegistry
{
public:
    /*!
     * Add a plugin to the registry at the specified path.
     * \param plugin the plugin to register
     */
    static void add(const Plugin &plugin);

    /*!
     * Convenience method to register a new plugin from components.
     */
    template <typename ValueType>
    static void add(const PluginPath &path, ValueType &&value);

    /*!
     * Convenience method to register a new callable plugin from components.
     * The second parameter must be a function pointer type.
     * The resulting plugin will contain a Pothos::Callable.
     */
    template <typename CallType>
    static void addCall(const PluginPath &path, const CallType &call);

    /*!
     * Remove a plugin from the registry at the specified path.
     * \throws PluginRegistryError if the path is not in the registry
     * \param path the path to the plugin in the registry
     * \return the removed Plugin
     */
    static Plugin remove(const PluginPath &path);

    /*!
     * Get a plugin from the registry at the specified path.
     * \throws PluginRegistryError if the path is not in the registry
     * \param path the path to the plugin in the registry
     * \return the Plugin at the specified path
     */
    static Plugin get(const PluginPath &path);

    /*!
     * Is there nothing located at the given path node?
     * Subtrees may have plugins, but this direct node may not.
     * \return true if the path node is empty
     */
    static bool empty(const PluginPath &path);

    /*!
     * Does the path exist in the registry?
     * \return true if the path exists
     */
    static bool exists(const PluginPath &path);

    /*!
     * List the PluginPath names at the given path.
     * The result will be in the order that the plugins were added.
     * \param path the base or directory path
     * \return a list of names in this path
     */
    static std::vector<std::string> list(const PluginPath &path);

    /*!
     * Create a dump structure of the entire registry.
     */
    static PluginRegistryInfoDump dump(void);

private:
    //! private constructor: we dont make PluginRegistry instances
    PluginRegistry(void){}
};

} //namespace Pothos

#include <utility> //std::forward

template <typename ValueType>
void Pothos::PluginRegistry::add(const PluginPath &path, ValueType &&value)
{
    Pothos::PluginRegistry::add(Pothos::Plugin(path, std::forward<ValueType>(value)));
}

#include <Pothos/Callable/CallableImpl.hpp> //used in template definition below:

template <typename CallType>
void Pothos::PluginRegistry::addCall(const PluginPath &path, const CallType &call)
{
    Pothos::PluginRegistry::add(Pothos::Plugin(path, Pothos::Callable(call)));
}
