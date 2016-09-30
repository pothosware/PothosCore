///
/// \file Plugin/Path.hpp
///
/// Plugin path represents a UNIX-style path for the plugin hierarchy.
///
/// \copyright
/// Copyright (c) 2013-2016 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Config.hpp>
#include <vector>
#include <string>

namespace Pothos {

/*!
 * Plugin path represents a UNIX-style path for the plugin hierarchy.
 * All paths must be absolute and start with the root slash.
 * Node names must be alphanumeric, underscored, hyphenated.
 * Example of a valid path: /foo_bar/my-module
 */
class POTHOS_API PluginPath
{
public:
    /*!
     * Create a PluginPath at the root ("/")
     */
    explicit PluginPath(void);

    /*!
     * Create a PluginPath from a path string.
     * A PluginPathError will be thrown on bad format.
     * \param path a plugin path in string representation
     */
    PluginPath(const std::string &path);

    /*!
     * Create a PluginPath by concatenating two paths.
     * The resulting path with be a valid /path0/path1
     */
    PluginPath(const PluginPath &path0, const PluginPath &path1);

    /*!
     * Create a PluginPath from a path string.
     * \throws PluginPathError if bad format.
     * \param path a plugin path in string representation
     */
    PluginPath(const char *path);

    /*!
     * Copy constructor for PluginPath.
     * \param path the PluginPath to copy
     */
    PluginPath(const PluginPath &path);

    /*!
     * Destructor for PluginPath.
     */
    ~PluginPath(void);

    /*!
     * Assignment operator for PluginPath.
     * \param path the PluginPath to assign to this
     */
    PluginPath &operator=(const PluginPath &path);

    /*!
     * Join a subpath with the path in this PluginPath.
     * \throws PluginPathError if bad format.
     * \param subPath the path to append to the end
     * \return a newly constructed PluginPath
     */
    PluginPath join(const std::string &subPath) const;

    /*!
     * List the nodes that make up the path.
     * Basically, this splits the path at the slashes.
     * \return a vector of strings between each slash
     */
    std::vector<std::string> listNodes(void) const;

    /*!
     * Get the PluginPath as a string representation.
     * \return the path as a string with slashes
     */
    std::string toString(void) const;

private:
    std::string _path;
};

/*!
 * Compare two plugin paths for equality.
 * \return true if the paths are identical
 */
POTHOS_API bool operator==(const PluginPath &lhs, const PluginPath &rhs);

} //namespace Pothos
