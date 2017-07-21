// Copyright (c) 2016-2017 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Plugin.hpp>
#include <Poco/Path.h>
#include <Poco/File.h>
#include <fstream>
#include <sstream>
#include <cassert>
#include <map>
#include <json.hpp>

using json = nlohmann::json;

/***********************************************************************
 * Load a JSON block description from file and register the descriptions.
 * return a list of registration paths and a list of paths for blocks.
 **********************************************************************/
static std::vector<Pothos::PluginPath> blockDescParser(std::istream &is, std::vector<Pothos::PluginPath> &blockPaths)
{
    std::vector<Pothos::PluginPath> entries;

    //parse the stream into a JSON array
    auto arrayOut = json::parse(is);
    if (arrayOut.is_object()) arrayOut = {arrayOut};
    for (const auto &obj : arrayOut)
    {
        const std::string JsonObjStr(obj.dump());

        std::vector<std::string> paths;
        paths.push_back(obj["path"].get<std::string>());
        if (obj.count("aliases")) for (const auto &alias : obj["aliases"])
        {
            paths.push_back(alias.get<std::string>());
        }

        //register the block description for every path
        for (const auto &path : paths)
        {
            const auto pluginPath = Pothos::PluginPath("/blocks/docs", path);
            Pothos::PluginRegistry::add(pluginPath, JsonObjStr);
            entries.push_back(pluginPath);
            blockPaths.push_back(Pothos::PluginPath("/blocks", path));
        }
    }
    return entries;
}

/***********************************************************************
 * Load a JSON block description described by a config file section
 **********************************************************************/
static std::vector<Pothos::PluginPath> blockDescLoader(const std::map<std::string, std::string> &config)
{
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

    //open an input file stream
    std::ifstream ifs(Poco::Path::expand(jsonPath.toString()));

    std::vector<Pothos::PluginPath> blockPaths;
    return blockDescParser(ifs, blockPaths);
}

/***********************************************************************
 * loader registration
 **********************************************************************/
pothos_static_block(pothosFrameworkRegisterBlockDescLoader)
{
    Pothos::PluginRegistry::addCall("/framework/conf_loader/block_desc", &blockDescLoader);
}
