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
#include <Pothos/Config.hpp>

namespace Pothos {

class POTHOS_API PluginLoader
{
public:

    /*!
     * Load all modules in the system install paths.
     */
    static void loadModules(void);

private:
    //! private constructor: we dont make PluginLoader instances
    PluginLoader(void){}
};

} //namespace Pothos
