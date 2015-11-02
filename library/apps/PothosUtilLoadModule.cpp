// Copyright (c) 2013-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PothosUtil.hpp"
#include <Pothos/Plugin/Module.hpp>
#include <Pothos/Plugin/Registry.hpp>
#include <iostream>
#include <cstdlib>

//remove outer quotes if they exist
static std::string unquote(const std::string &s)
{
    if (s.size() > 2 and s.front() == '"' and s.back() == '"')
    {
        return s.substr(1, s.length()-2);
    }
    else return s;
}

void PothosUtilBase::loadModule(const std::string &, const std::string &path)
{
    std::cout << "Loading: " << path << std::endl;
    //this tests module load and unload -- could throw or cause abort
    {
        Pothos::PluginModule module(unquote(path));
        for (std::string pluginPath : module.getPluginPaths())
        {
            Pothos::PluginRegistry::remove(pluginPath);
        }
    }
    std::cout << "success!" << std::endl;
}
