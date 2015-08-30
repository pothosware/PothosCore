// Copyright (c) 2013-2013 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Plugin/Exception.hpp>
#include <typeinfo>

namespace Pothos
{
POTHOS_IMPLEMENT_EXCEPTION(PluginPathError, RuntimeException, "Plugin Path Error")
POTHOS_IMPLEMENT_EXCEPTION(PluginRegistryError, RuntimeException, "Plugin Registry Error")
POTHOS_IMPLEMENT_EXCEPTION(PluginModuleError, RuntimeException, "Plugin Module Error")
} //namespace Pothos
