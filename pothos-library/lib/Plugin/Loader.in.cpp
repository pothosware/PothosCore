// Copyright (c) 2013-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Plugin/Loader.hpp>
#include <Pothos/System.hpp>
#include <Pothos/Plugin.hpp>
#include <Poco/StringTokenizer.h>
#include <Poco/SharedLibrary.h>
#include <Poco/Environment.h>
#include <Poco/Logger.h>
#include <Poco/Path.h>
#include <Poco/File.h>
#include <future>

static void loadModuleAtPath(const std::string &path)
{
    try
    {
        Pothos::PluginModule::safeLoad(path);
    }
    catch(const Pothos::PluginModuleError &ex)
    {
        poco_error(Poco::Logger::get("Pothos.PluginLoader.load"), ex.displayText());
    }
}

std::vector<Poco::Path> getModulePaths(const Poco::Path &path)
{
    poco_information(Poco::Logger::get("Pothos.PluginLoader.load"), path.toString());

    std::vector<Poco::Path> paths;

    const Poco::File file(path);
    if (not file.exists()) return paths;
    else if (file.isFile() and (path.getExtension() == "@MODULE_EXT@"))
    {
        paths.push_back(path);
    }
    else if (file.isDirectory())
    {
        std::vector<std::string> files; file.list(files);
        for (size_t i = 0; i < files.size(); i++)
        {
            auto subpaths = getModulePaths(Poco::Path(path, files[i]).absolute());
            paths.insert(paths.end(), subpaths.begin(), subpaths.end());
        }
    }

    return paths;
}

void Pothos::PluginLoader::loadModules(void)
{
    Poco::Path libPath = Pothos::System::getRootPath();
    libPath.append("lib@LIB_SUFFIX@");
    libPath.append("Pothos");
    libPath.append("modules");
    auto searchPaths = getModulePaths(libPath.absolute());

    //separator for search paths
    #ifdef _MSC_VER
    static const std::string sep = ";";
    #else
    static const std::string sep = ":";
    #endif

    //check the environment's search path
    const auto pluginPaths = Poco::Environment::get("POTHOS_PLUGIN_PATH", "");
    for (const auto &pluginPath : Poco::StringTokenizer(pluginPaths, sep))
    {
        if (pluginPath.empty()) continue;
        const auto subSearchPaths = getModulePaths(Poco::Path(pluginPath).absolute());
        searchPaths.insert(searchPaths.end(), subSearchPaths.begin(), subSearchPaths.end());
    }

    //spawn futures and wait for completion of load
    std::vector<std::future<void>> futures;
    for (const auto &path : searchPaths)
    {
        futures.push_back(std::async(std::launch::async, &loadModuleAtPath, path.toString()));
    }
    for (auto &future : futures) future.wait();
}
