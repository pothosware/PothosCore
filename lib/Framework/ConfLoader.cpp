// Copyright (c) 2016-2016 Josh Blum
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

static std::vector<std::string> loadConfFile(const std::string &path)
{
    return std::vector<std::string>();
}

static std::vector<Poco::Path> getConfFilePaths(const Poco::Path &path)
{
    poco_debug(Poco::Logger::get("Pothos.ConfLoader.load"), path.toString());

    std::vector<Poco::Path> paths;

    const Poco::File file(path);
    if (not file.exists()) return paths;
    else if (file.isFile() and (path.getExtension() == "conf"))
    {
        paths.push_back(path);
    }
    else if (file.isDirectory())
    {
        std::vector<std::string> files; file.list(files);
        for (size_t i = 0; i < files.size(); i++)
        {
            auto subpaths = getConfFilePaths(Poco::Path(path, files[i]).absolute());
            paths.insert(paths.end(), subpaths.begin(), subpaths.end());
        }
    }

    return paths;
}

std::vector<std::string> Pothos_ConfLoader_loadConfFiles(void)
{
    //the default search path
    std::vector<Poco::Path> searchPaths;
    Poco::Path confPath = Pothos::System::getRootPath();
    confPath.append("share");
    confPath.append("Pothos");
    confPath.append("blocks");
    searchPaths.push_back(confPath);

    //the user's home directory path
    confPath = Pothos::System::getUserConfigPath();
    confPath.append("blocks");
    searchPaths.push_back(confPath);

    //separator for search paths
    const std::string sep(1, Poco::Path::pathSeparator());

    //check the environment's search path
    const auto confPaths = Poco::Environment::get("POTHOS_CONF_PATH", "");
    for (const auto &confPath : Poco::StringTokenizer(confPaths, sep))
    {
        if (confPath.empty()) continue;
        searchPaths.push_back(Poco::Path(confPath));
    }

    //traverse the search paths and spawn futures
    std::vector<std::future<std::vector<std::string>>> futures;
    for (const auto &searchPath : searchPaths)
    {
        for (const auto &path : getConfFilePaths(searchPath.absolute()))
        {
            futures.push_back(std::async(std::launch::async, &loadConfFile, path.toString()));
        }
    }

    //wait for completion of future module load
    std::vector<std::string> entries;
    for (auto &future : futures)
    {
        POTHOS_EXCEPTION_TRY
        {
            auto entries_i = future.get();
            entries.insert(entries.end(), entries_i.begin(), entries_i.end());
        }
        POTHOS_EXCEPTION_CATCH (const Pothos::Exception &ex)
        {
            poco_error(Poco::Logger::get("Pothos.ConfLoader.load"), ex.displayText());
        }
    }
    return entries;
}
