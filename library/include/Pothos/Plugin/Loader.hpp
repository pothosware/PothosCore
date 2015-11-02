///
/// \file Plugin/Loader.hpp
///
/// The loader is responsible for loading runtime modules into the plugin registry.
///
/// \copyright
/// Copyright (c) 2013-2014 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Plugin/Module.hpp>
#include <Pothos/Config.hpp>
#include <vector>

namespace Pothos {

class POTHOS_API PluginLoader
{
public:

    /*!
     * Load all modules in the system install paths.
     * They will not be unloaded so long as the returned vector is not destroyed.
     */
    static std::vector<Pothos::PluginModule> loadModules(void);

private:
    //! private constructor: we dont make PluginLoader instances
    PluginLoader(void){}
};

} //namespace Pothos
