// Copyright (c) 2020 NIcholas Corgan
// SPDX-License-Identifier: BSL-1.0

#include "PothosUtil.hpp"

#include <Pothos/Plugin.hpp>

#include <algorithm>
#include <cassert>
#include <iomanip>
#include <iostream>
#include <map>
#include <string>

static std::map<std::string, std::string> modVers;
static std::map<std::string, size_t> modNumsPlugins;
static size_t maxPathLength = 0;

static void traverseDump(const Pothos::PluginRegistryInfoDump& dump)
{
    if(!dump.modulePath.empty())
    {
        if(modNumsPlugins.count(dump.modulePath) > 0)
        {
            assert(modVers.count(dump.modulePath) > 0);
            ++modNumsPlugins[dump.modulePath];
        }
        else
        {
            modVers[dump.modulePath] = dump.moduleVersion;
            modNumsPlugins[dump.modulePath] = 1;

            maxPathLength = std::max(maxPathLength, dump.modulePath.size());
        }
    }
    for(const auto& sub: dump.subInfo)
    {
        traverseDump(sub);
    }
}

void PothosUtilBase::listModules(const std::string&, const std::string&)
{
    Pothos::ScopedInit init;

    traverseDump(Pothos::PluginRegistry::dump());

    using MapPair = std::map<std::string, size_t>::value_type;
    const auto maxNumPluginsIter = std::max_element(
                                       modNumsPlugins.begin(),
                                       modNumsPlugins.end(),
                                       [](const MapPair& p1, const MapPair& p2)
                                       {
                                           return (p1.second < p2.second);
                                       });
    const auto maxNumPluginsLength = std::to_string(maxNumPluginsIter->second).size();

    for(const auto& modVersMapPair: modVers)
    {
        const auto& modulePath = modVersMapPair.first;

        std::cout << std::left << std::setw(maxPathLength) << modulePath << " ("
                  << std::right << std::setw(maxNumPluginsLength) << modNumsPlugins[modulePath]
                  << " plugins) [" << modVersMapPair.second << "]" << std::endl;
    }
}
