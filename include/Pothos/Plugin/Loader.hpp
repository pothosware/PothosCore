///
/// \file Plugin/Loader.hpp
///
/// The loader is responsible for loading runtime modules into the plugin registry.
///
/// \copyright
/// Copyright (c) 2013-2015 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Plugin/Module.hpp>
#include <vector>

namespace Pothos {

class POTHOS_API PluginLoader
{
public:

    /*!
     * Load all modules in the system install paths.
     * The caller should hold onto the module handles.
     * Releasing the handles will unload the plugins.
     * \return a list of loaded module handles
     */
    static std::vector<PluginModule> loadModules(void);

private:
    //! private constructor: we dont make PluginLoader instances
    PluginLoader(void){}
};

} //namespace Pothos
