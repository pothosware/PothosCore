//
// Plugin/Exception.hpp
//
// Exceptions thrown by the Plugin methods.
//
// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0
//

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Exception.hpp>

namespace Pothos {

/*!
 * A PluginPathError is thrown when bad plugin paths are used.
 */
POTHOS_DECLARE_EXCEPTION(POTHOS_API, PluginPathError, RuntimeException)

/*!
 * A PluginRegistryError is thrown when bad registry paths are used.
 */
POTHOS_DECLARE_EXCEPTION(POTHOS_API, PluginRegistryError, RuntimeException)

/*!
 * A PluginModuleError is thrown when module loads fail.
 */
POTHOS_DECLARE_EXCEPTION(POTHOS_API, PluginModuleError, RuntimeException)

} //namespace Pothos
