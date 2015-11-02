// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Plugin/Module.hpp>
#include <Pothos/Plugin/Plugin.hpp>
#include <Poco/RWLock.h>
#include <Poco/HashMap.h>
#include <Poco/SingletonHolder.h>

static Poco::RWLock &getModulePathsMutex(void)
{
    static Poco::SingletonHolder<Poco::RWLock> sh;
    return *sh.get();
}

typedef Poco::HashMap<std::string, std::vector<std::string>> ModulePathsMap;
static ModulePathsMap &getModulePathsMap(void)
{
    static Poco::SingletonHolder<ModulePathsMap> sh;
    return *sh.get();
}

std::vector<std::string> Pothos::PluginModule::getModulePluginPaths(const std::string &filePath)
{
    Poco::RWLock::ScopedReadLock lock(getModulePathsMutex());
    return getModulePathsMap()[filePath];
}

void updatePluginAssociation(const std::string &action, const Pothos::Plugin &plugin)
{
    Poco::RWLock::ScopedWriteLock lock(getModulePathsMutex());
    auto &v = getModulePathsMap()[plugin.getModulePath()];
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
