// Copyright (c) 2013-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PothosUtil.hpp"
#include <Pothos/Plugin.hpp>
#include <iostream>

static void printPluginTreeR(const Pothos::PluginPath &path)
{
    Pothos::Plugin plugin(path);
    if (not Pothos::PluginRegistry::empty(path))
    {
        plugin = Pothos::PluginRegistry::get(path);
    }
    std::cout << plugin.toString() << std::endl;

    //iterate on the subtree stuff
    auto nodes = Pothos::PluginRegistry::list(path);
    for (auto it = nodes.begin(); it != nodes.end(); it++)
    {
        printPluginTreeR(path.join(*it));
    }
}

void PothosUtilBase::printPluginTree(const std::string &, const std::string &pluginPath)
{
    Pothos::ScopedInit init;
    printPluginTreeR(pluginPath.empty()?"/":pluginPath);
    std::cout << std::endl;
}
