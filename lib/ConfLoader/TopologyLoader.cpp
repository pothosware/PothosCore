// Copyright (c) 2016-2016 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Plugin.hpp>
#include <Poco/Path.h>
#include <Poco/File.h>
#include <fstream>
#include <map>

//TODO not finished, factory args implementation....

/***********************************************************************
 * Load a JSON topology described by a config file section
 **********************************************************************/
static std::vector<Pothos::PluginPath> topologyLoader(const std::map<std::string, std::string> &config)
{
    std::vector<Pothos::PluginPath> entries;

    //config file path set by caller
    const auto confFilePathIt = config.find("confFilePath");
    if (confFilePathIt == config.end() or confFilePathIt->second.empty())
        throw Pothos::Exception("missing confFilePath");

    //determine JSON description file path
    const auto jsonIt = config.find("json");
    if (jsonIt == config.end() or jsonIt->second.empty())
        throw Pothos::Exception("JSON file not specified");
    Poco::Path jsonPath(jsonIt->second);
    jsonPath.makeAbsolute(Poco::Path(confFilePathIt->second).makeParent());
    if (not Poco::File(jsonPath).exists())
        throw Pothos::Exception(jsonPath.toString() + " does not exist");

    //get the plugin path
    const auto pathIt = config.find("path");
    if (pathIt == config.end() or pathIt->second.empty())
        throw Pothos::Exception("missing plugin path");
    const auto pluginPath = Pothos::PluginPath("/blocks").join(pathIt->second.substr(1));

    //parse the file into a string
    std::ifstream ifs(Poco::Path::expand(jsonPath.toString()));
    const std::string json((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

    //create an entry for the factory
    //not ready yet! need factory args list in design
    //Pothos::PluginRegistry::addCall(pluginPath, &Pothos::Topology::make

    entries.push_back(pluginPath);
    return entries;
}

/***********************************************************************
 * loader registration
 **********************************************************************/
pothos_static_block(pothosFrameworkRegisterTopologyLoader)
{
    Pothos::PluginRegistry::addCall("/framework/conf_loader/Topology", &topologyLoader);
}
