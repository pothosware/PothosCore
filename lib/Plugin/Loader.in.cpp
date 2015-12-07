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

std::vector<Pothos::PluginModule> Pothos::PluginLoader::loadModules(void)
{
    //the default search path
    std::vector<Poco::Path> searchPaths;
    Poco::Path libPath = Pothos::System::getRootPath();
    libPath.append("lib@LIB_SUFFIX@");
    libPath.append("Pothos");
    libPath.append("modules");
    searchPaths.push_back(libPath);

    //support /usr/local module installs when the install prefix is /usr
    if (Pothos::System::getRootPath() == "/usr")
    {
        searchPaths.push_back("/usr/local/lib@LIB_SUFFIX@/Pothos/modules");
        //when using a multi-arch directory, support single-arch path as well
        static const std::string libsuffix("@LIB_SUFFIX@");
        if (not libsuffix.empty() and libsuffix.at(0) == '/')
            searchPaths.push_back("/usr/local/lib/Pothos/modules");

    }

    //separator for search paths
    const std::string sep(1, Poco::Path::pathSeparator());

    //check the environment's search path
    const auto pluginPaths = Poco::Environment::get("POTHOS_PLUGIN_PATH", "");
    for (const auto &pluginPath : Poco::StringTokenizer(pluginPaths, sep))
    {
        if (pluginPath.empty()) continue;
        searchPaths.push_back(Poco::Path(pluginPath));
    }

    //traverse the search paths and spawn futures
    std::vector<std::future<Pothos::PluginModule>> futures;
    for (const auto &searchPath : searchPaths)
    {
        for (const auto &path : getModulePaths(searchPath.absolute()))
        {
            futures.push_back(std::async(std::launch::async, &Pothos::PluginModule::safeLoad, path.toString()));
        }
    }

    //wait for completion of future module load
    std::vector<PluginModule> modules;
    for (auto &future : futures)
    {
        POTHOS_EXCEPTION_TRY
        {
            modules.push_back(future.get());
        }
        POTHOS_EXCEPTION_CATCH (const Pothos::Exception &ex)
        {
            poco_error(Poco::Logger::get("Pothos.PluginLoader.load"), ex.displayText());
        }
    }
    return modules;
}
