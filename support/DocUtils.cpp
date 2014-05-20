// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Exception.hpp>
#include <Pothos/Plugin.hpp>
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Parser.h>
#include <sstream>

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
            Poco::JSON::Parser p;
            p.parse(plugin.getObject().extract<std::string>());
            array->add(p.getHandler()->asVar());
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
    static std::string dump(void)
    {
        std::ostringstream outStream;
        recurseParseDocPath("/blocks/docs")->stringify(outStream);
        return outStream.str();
    }
};

#include <Pothos/Managed.hpp>

static auto managedDocUtils = Pothos::ManagedClass()
    .registerClass<DocUtilsDumpJson>()
    .registerStaticMethod("dumpJson", &DocUtilsDumpJson::dump)
    .commit("Pothos/Gui/DocUtils");
