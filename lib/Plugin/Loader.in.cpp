// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Plugin/Loader.hpp>
#include <Pothos/System.hpp>
#include <Pothos/Plugin.hpp>
#include <Poco/SharedLibrary.h>
#include <Poco/Environment.h>
#include <Poco/Logger.h>
#include <Poco/Path.h>
#include <Poco/File.h>

void loadModuleAtPath(const Poco::Path &path)
{
    poco_information(Poco::Logger::get("Pothos.PluginLoader.load"), path.toString());

    const Poco::File file(path);
    if (not file.exists()) return;
    else if (file.isFile() and (path.getExtension() == "@MODULE_EXT@"))
    {
        try
        {
            Pothos::PluginModule::safeLoad(path.toString());
        }
        catch(const Pothos::PluginModuleError &ex)
        {
            poco_error(Poco::Logger::get("Pothos.PluginLoader.load"), ex.displayText());
        }
    }
    else if (file.isDirectory())
    {
        std::vector<std::string> files; file.list(files);
        for (size_t i = 0; i < files.size(); i++)
        {
            loadModuleAtPath(Poco::Path(path, files[i]).absolute());
        }
    }
}

void Pothos::PluginLoader::loadModules(void)
{
    Poco::Path libPath = Pothos::System::getRootPath();
    libPath.append("lib@LIB_SUFFIX@");
    libPath.append("Pothos");
    libPath.append("modules");
    loadModuleAtPath(libPath.absolute());

    //TODO load user built modules -- when we have a comprehension for them
}
