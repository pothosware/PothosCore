// Copyright (c) 2013-2016 Josh Blum
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

static std::vector<Poco::Path> getModulePaths(const Poco::Path &path)
{
    poco_debug(Poco::Logger::get("Pothos.PluginLoader.load"), path.toString());

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
    const auto searchPaths = Pothos::System::getPothosModuleSearchPaths();

    //traverse the search paths and spawn futures
    std::vector<std::future<Pothos::PluginModule>> futures;
    for (const auto &searchPath : searchPaths)
    {
        for (const auto &path : getModulePaths(searchPath))
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
