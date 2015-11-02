///
/// \file Plugin/Plugin.hpp
///
/// The plugin representation for the plugin registry.
///
/// \copyright
/// Copyright (c) 2013-2014 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Plugin/Path.hpp>
#include <Pothos/Plugin/Module.hpp>
#include <Pothos/Object/Object.hpp>
#include <string>

namespace Pothos {

/*!
 * A plugin is a combination of an Path, Object, and a Module.
 */
class POTHOS_API Plugin
{
public:
    //! Create a null plugin
    Plugin(void);

    /*!
     * Create a plugin from components.
     */
    Plugin(const PluginPath &path,
        const Object &object = Object(),
        const std::string &modulePath = "");

    /*!
     * Create a plugin from components.
     * This templated version takes any type and stores it in an Object.
     */
    template <typename ValueType>
    Plugin(const PluginPath &path, ValueType &&value);

    //! Get the path from the Plugin.
    const PluginPath &getPath(void) const;

    //! Get the object from the Plugin.
    const Object &getObject(void) const;

    //! Get the module path from the Plugin.
    const std::string &getModulePath(void) const;

    //! String representation of plugin
    std::string toString(void) const;

private:
    PluginPath _path;
    Object _object;
    std::string _modulePath;
};

} //namespace Pothos

#include <utility> //std::forward

template <typename ValueType>
Pothos::Plugin::Plugin(const PluginPath &path, ValueType &&value):
    _path(path),
    _object(Object(std::forward<ValueType>(value)))
{
    return;
}
