// Copyright (c) 2018-2018 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PothosUtil.hpp"
#include <Pothos/Plugin.hpp>
#include <map>
#include <algorithm>
#include <iostream>

static std::map<std::string, std::vector<std::string>> modMap;

static void traverseDump(const Pothos::PluginRegistryInfoDump &dump)
{
    if (not dump.objectType.empty() and not dump.modulePath.empty())
    {
        modMap[dump.modulePath].push_back(dump.pluginPath);
    }

    for (const auto &subInfo : dump.subInfo)
    {
        traverseDump(subInfo);
    }
}

void PothosUtilBase::printModuleInfo(const std::string &, const std::string &)
{
    Pothos::ScopedInit init;
    traverseDump(Pothos::PluginRegistry::dump());
    size_t maxPathLength(0);
    for (const auto &module : modMap)
    {
        maxPathLength = std::max(maxPathLength, module.first.size());
    }

    for (const auto &module : modMap)
    {
        const auto paddedName = module.first + std::string(maxPathLength-module.first.size()+1, ' ');
        std::cout << paddedName << "(" << module.second.size() << " plugins)" << std::endl;
        /*for (const auto &pluginPath : module.second)
        {
            std::cout << "  " << pluginPath << std::endl;
        }
        */
    }
    std::cout << std::endl;
}
