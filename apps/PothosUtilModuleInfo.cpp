// Copyright (c) 2018-2018 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PothosUtil.hpp"
#include <Pothos/Plugin.hpp>
#include <map>
#include <algorithm>
#include <iostream>
#include <iomanip>

static std::map<std::string, std::vector<std::string>> modMap;
static std::map<std::string, std::string> modVers;

static void traverseDump(const std::string &modulePath, const Pothos::PluginRegistryInfoDump &dump)
{
    const bool isBuiltin = modulePath == "builtin" and dump.modulePath.empty();
    const bool filterPass = modulePath.empty() or modulePath == dump.modulePath;
    if (not dump.objectType.empty() and (isBuiltin or filterPass))
    {
        modMap[dump.modulePath].push_back(dump.pluginPath);
        modVers[dump.modulePath] = dump.moduleVersion;
    }

    for (const auto &subInfo : dump.subInfo)
    {
        traverseDump(modulePath, subInfo);
    }
}

static const std::string stringifyName(const std::string &modulePath)
{
    return modulePath.empty()?"Builtin":modulePath;
}

void PothosUtilBase::printModuleInfo(const std::string &, const std::string &modulePath)
{
    Pothos::ScopedInit init;
    traverseDump(modulePath, Pothos::PluginRegistry::dump());
    size_t maxPathLength(0);
    for (const auto &module : modMap)
    {
        const auto displayName = stringifyName(module.first);
        maxPathLength = std::max(maxPathLength, displayName.size());
    }

    for (const auto &module : modMap)
    {
        const auto displayName = stringifyName(module.first);
        const auto paddedName = displayName + std::string(maxPathLength-displayName.size()+1, ' ');
        std::cout << paddedName << "(" << std::setw(2) << module.second.size() << " plugins)";
        const auto modVer = modVers.at(module.first);
        if (not modVer.empty()) std::cout << " [" << modVer << "]";
        std::cout << std::endl;
        if (not modulePath.empty())
        {
            for (const auto &pluginPath : module.second)
            {
                std::cout << "  " << pluginPath << std::endl;
            }
        }
    }
    std::cout << std::endl;
}
