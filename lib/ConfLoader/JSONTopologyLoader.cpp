// Copyright (c) 2016-2016 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Plugin.hpp>
#include <Pothos/Framework.hpp>
#include <Poco/Path.h>
#include <Poco/File.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Parser.h>
#include <sstream>
#include <fstream>
#include <map>

/***********************************************************************
 * The topology factory loads args into a JSON description
 * and creates and returns an instance of the topology
 **********************************************************************/
static Pothos::Object opaqueJSONTopologyFactory(
    const std::string &json,
    const Pothos::Object *args,
    const size_t numArgs)
{
    //parse the json formatted string into a JSON object
    Poco::JSON::Object::Ptr topObj;
    try
    {
        const auto result = Poco::JSON::Parser().parse(json);
        topObj = result.extract<Poco::JSON::Object::Ptr>();
    }
    catch (const Poco::Exception &ex)
    {
        throw Pothos::DataFormatException(ex.message());
    }

    //apply the global variable overlays
    if (not topObj->isArray("globals")) topObj->set("globals",
        Poco::JSON::Array::Ptr(new Poco::JSON::Array()));
    auto globalsArray = topObj->getArray("globals");
    if (numArgs > globalsArray->size())
    {
        throw Pothos::InvalidArgumentException("too many args passed to factory");
    }

    for (size_t i = 0; i < numArgs; i++)
    {
        if (not globalsArray->isObject(i))
        {
            throw Pothos::DataFormatException("global entry not a JSON object");
        }
        auto globalVarObj = globalsArray->getObject(i);
        globalVarObj->set("value", args[i].toString());
    }

    //create the topology from the JSON string
    std::stringstream ss; topObj->stringify(ss);
    return Pothos::Object(Pothos::Topology::make(ss.str()));
}

/***********************************************************************
 * Load a JSON topology described by a config file section
 **********************************************************************/
static std::vector<Pothos::PluginPath> JSONTopologyLoader(const std::map<std::string, std::string> &config)
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
    const auto pluginPath = Pothos::PluginPath("/blocks", pathIt->second);

    //parse the file into a string
    std::ifstream ifs(Poco::Path::expand(jsonPath.toString()));
    const std::string json((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

    //create an entry for the factory
    const auto factory = Pothos::Callable(&opaqueJSONTopologyFactory).bind(json, 0);
    Pothos::PluginRegistry::addCall(pluginPath, factory);

    entries.push_back(pluginPath);
    return entries;
}

/***********************************************************************
 * loader registration
 **********************************************************************/
pothos_static_block(pothosFrameworkRegisterJSONTopologyLoader)
{
    Pothos::PluginRegistry::addCall("/framework/conf_loader/json_topology", &JSONTopologyLoader);
}
