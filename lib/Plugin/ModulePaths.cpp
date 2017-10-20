// Copyright (c) 2013-2017 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Plugin/Module.hpp>
#include <Pothos/Plugin/Plugin.hpp>
#include <Pothos/Util/SpinLockRW.hpp>
#include <Poco/HashMap.h>
#include <mutex>

static Pothos::Util::SpinLockRW &getModulePathsMutex(void)
{
    static Pothos::Util::SpinLockRW lock;
    return lock;
}

typedef Poco::HashMap<std::string, std::vector<std::string>> ModulePathsMap;
static ModulePathsMap &getModulePathsMap(void)
{
    static ModulePathsMap map;
    return map;
}

std::vector<std::string> getPluginPaths(const Pothos::PluginModule &module)
{
    Pothos::Util::SpinLockRW::SharedLock lock(getModulePathsMutex());
    return getModulePathsMap()[module.getFilePath()];
}

void updatePluginAssociation(const std::string &action, const Pothos::Plugin &plugin)
{
    std::lock_guard<Pothos::Util::SpinLockRW> lock(getModulePathsMutex());
    auto &v = getModulePathsMap()[plugin.getModule().getFilePath()];
    const auto &path = plugin.getPath().toString();
    if (action == "add")
    {
        v.push_back(path);
    }
    if (action == "remove")
    {
        auto it = std::find(v.begin(), v.end(), path);
        if (it != v.end()) v.erase(it);
    }
}
