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
#include <Poco/Format.h>
#include <Poco/Util/IniFileConfiguration.h>
#include <future>
#include <fstream>
#include <iostream>

static Poco::Logger &confLoaderLogger(void)
{
    static Poco::Logger &logger(Poco::Logger::get("Pothos.ConfLoader"));
    return logger;
}

static std::vector<std::string> loadConfFile(const std::string &path)
{
    poco_debug_f1(confLoaderLogger(), "loading %s", path);

    std::vector<std::string> entries;
    Poco::AutoPtr<Poco::Util::IniFileConfiguration> conf(new Poco::Util::IniFileConfiguration(path));
    Poco::Util::AbstractConfiguration::Keys rootKeys; conf->keys(rootKeys);
    for (const auto &rootKey : rootKeys)
    {
        poco_debug_f2(confLoaderLogger(), "loading %s[%s]", path, rootKey);

        //get the plugin path
        if (not conf->hasProperty(rootKey+".path")) throw Pothos::Exception(
            Poco::format("%s[%s] does not specify a path", path, rootKey));
        const auto pluginPath = Pothos::PluginPath(conf->getString(rootKey+".path"));

        //get the loader type
        if (not conf->hasProperty(rootKey+".loader")) throw Pothos::Exception(
            Poco::format("%s[%s] does not specify a loader", path, rootKey));
        const std::string loader(conf->getString(rootKey+".loader"));
        //TODO does loader exist?

        //create a mapping of the config data
        Poco::Util::AbstractConfiguration::Keys subKeys; conf->keys(rootKey, subKeys);
        std::map<std::string, std::string> subConfig;
        for (const auto &subKey : subKeys)
        {
            subConfig[subKey] = conf->getString(rootKey+"."+subKey);
        }

        //load optional JSON description from file
        if (conf->hasProperty(rootKey+".description"))
        {
            Poco::Path descPath(conf->getString(rootKey+".description"));
            descPath.makeAbsolute(Poco::Path(path).makeParent());
            if (not Poco::File(descPath).exists()) throw Pothos::Exception(
            Poco::format("%s[%s] description path %s does not exist", path, rootKey, descPath.toString()));
            std::ifstream ifs(Poco::Path::expand(descPath.toString()));
            const std::string desc((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

            //register the contents into the plugin registry
            const Pothos::PluginPath descPluginPath("/blocks/docs"+pluginPath.toString());
            Pothos::PluginRegistry::add(descPluginPath, desc);
            entries.push_back(descPluginPath.toString());
        }
    }
    return entries;
}

static std::vector<Poco::Path> getConfFilePaths(const Poco::Path &path)
{
    poco_debug_f1(confLoaderLogger(), "traversing %s", path.toString());

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

    //the user's home config path
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
            poco_error(confLoaderLogger(), ex.displayText());
        }
    }
    return entries;
}
