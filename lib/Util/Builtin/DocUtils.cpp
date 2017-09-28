// Copyright (c) 2014-2017 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Exception.hpp>
#include <Pothos/Plugin.hpp>
#include <string>
#include <sstream>

static void recurseParseDocPath(const Pothos::PluginPath &path, std::stringstream &array, size_t &count)
{
    //extract the plugin
    Pothos::Plugin plugin;
    try
    {
        plugin = Pothos::PluginRegistry::get(path);
    }
    catch (const Pothos::PluginRegistryError &)
    {
        //OK to be missing a plugin in a directory
    }

    //if the type is a string, try to parse it into a json array
    if (plugin.getObject().type() == typeid(std::string))
    {
        if (count++ != 0) array << ",";
        array << plugin.getObject().extract<std::string>();
    }

    //iterate over subdirectories
    for (const auto &dir : Pothos::PluginRegistry::list(path))
    {
        recurseParseDocPath(path.join(dir), array, count);
    }
}

class DocUtilsDumpJson
{
public:
    static std::string dumpJson(void)
    {
        size_t count(0);
        std::stringstream array;
        array << "[";
        recurseParseDocPath("/blocks/docs", array, count);
        array << "]";
        return array.str();
    }
    static std::string dumpJsonAt(const std::string &path)
    {
        auto plugin = Pothos::PluginRegistry::get("/blocks/docs"+path);
        return plugin.getObject().operator std::string();
    }
};

#include <Pothos/Managed.hpp>

static auto managedDocUtils = Pothos::ManagedClass()
    .registerClass<DocUtilsDumpJson>()
    .registerStaticMethod(POTHOS_FCN_TUPLE(DocUtilsDumpJson, dumpJson))
    .registerStaticMethod(POTHOS_FCN_TUPLE(DocUtilsDumpJson, dumpJsonAt))
    .commit("Pothos/Util/DocUtils");
