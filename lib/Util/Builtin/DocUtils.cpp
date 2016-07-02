// Copyright (c) 2014-2016 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Exception.hpp>
#include <Pothos/Plugin.hpp>
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Parser.h>

static Poco::JSON::Array::Ptr recurseParseDocPath(const Pothos::PluginPath &path)
{
    Poco::JSON::Array::Ptr array(new Poco::JSON::Array());

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
        try
        {
            const auto str = plugin.getObject().extract<std::string>();
            array->add(Poco::JSON::Parser().parse(str));
        }
        catch (const Poco::Exception &ex)
        {
            throw Pothos::SyntaxException("DocUtils::recurseParseDocPath("+path.toString()+")", ex.displayText());
        }
    }

    //iterate over subdirectories
    for (const auto &dir : Pothos::PluginRegistry::list(path))
    {
        const auto subArray = recurseParseDocPath(path.join(dir));
        for (size_t sai = 0; sai < subArray->size(); sai++)
        {
            array->add(subArray->get(sai));
        }
    }

    return array;
}

class DocUtilsDumpJson
{
public:
    static Poco::JSON::Array::Ptr dumpJson(void)
    {
        return recurseParseDocPath("/blocks/docs");
    }
    static Poco::JSON::Object::Ptr dumpJsonAt(const std::string &path)
    {
        auto plugin = Pothos::PluginRegistry::get("/blocks/docs"+path);
        auto obj = plugin.getObject();
        const auto result = Poco::JSON::Parser().parse(obj.convert<std::string>());
        return result.extract<Poco::JSON::Object::Ptr>();
    }
};

#include <Pothos/Managed.hpp>

static auto managedDocUtils = Pothos::ManagedClass()
    .registerClass<DocUtilsDumpJson>()
    .registerStaticMethod(POTHOS_FCN_TUPLE(DocUtilsDumpJson, dumpJson))
    .registerStaticMethod(POTHOS_FCN_TUPLE(DocUtilsDumpJson, dumpJsonAt))
    .commit("Pothos/Util/DocUtils");
