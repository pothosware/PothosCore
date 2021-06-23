///
/// \file Plugin/Module.hpp
///
/// A PluginModule interacts with loadable library modules and the plugin registry.
///
/// \copyright
/// Copyright (c) 2013-2018 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Config.hpp>
#include <memory>
#include <vector>
#include <string>

namespace Pothos {

class Plugin;

/*!
 * PluginModule represents a loaded shared library in the filesystem.
 */
class POTHOS_API PluginModule
{
public:
    //! A reference to a permanent empty module
    static const PluginModule &null(void);

    /*!
     * Create a null PluginModule.
     */
    PluginModule(void);

    /*!
     * Create a PluginModule from a file path.
     * \throw PluginPluginModuleError if the path does not exist
     * \throw PluginPluginModuleError if the load fails
     * \param path the path to a loadable module on the file system
     */
    PluginModule(const std::string &path);

    //! Plugin module destructor.
    ~PluginModule(void);

    /*!
     * Test load this library module within a separate process
     * to avoid loading something destructive within this process.
     * If the test load succeeds, the module will be loaded locally.
     * \throw PluginPluginModuleError if the load fails
     * \param path the path to a loadable module on the file system
     * \return the PluginModule loaded at the given file path
     */
    static PluginModule safeLoad(const std::string &path);

    /*!
     * Get a file path for this module.
     * \return the file path for the shared library
     */
    std::string getFilePath(void) const;

    /*!
     * Get the paths that this module loaded into the plugin registry.
     * Each path is a string that represents a path in the registry.
     */
    const std::vector<std::string> &getPluginPaths(void) const;

    /*!
     * Does the module hold a loaded library?
     * \return true if the module is non-empty
     */
    explicit operator bool(void) const;

    /*!
     * Get a version string for the specified module.
     * Modules may optionally provide version strings.
     * \return a version string or empty if no version provided
     */
    std::string getVersion(void) const;

private:
    struct Impl;
    std::shared_ptr<Impl> _impl;
    friend Plugin;
};

//! \cond
//! Internal call to register version with a module during load
class POTHOS_API ModuleVersion
{
public:
    ModuleVersion(const std::string &version);
};
//! \endcond

} //namespace Pothos
