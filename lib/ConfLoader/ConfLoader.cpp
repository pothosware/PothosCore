// Copyright (c) 2016-2016 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/System.hpp>
#include <Pothos/Plugin.hpp>
#include <Poco/StringTokenizer.h>
#include <Poco/SharedLibrary.h>
#include <Poco/Environment.h>
#include <Poco/Logger.h>
#include <Poco/Path.h>
#include <Poco/File.h>
#include <Poco/Format.h>
#include <Poco/Util/PropertyFileConfiguration.h>
#include <future>
#include <map>

static Poco::Logger &confLoaderLogger(void)
{
    static Poco::Logger &logger(Poco::Logger::get("Pothos.ConfLoader"));
    return logger;
}

std::string Pothos_FileRealPath(const std::string &path);

/***********************************************************************
 * Load a config file by iterating through sections and running action
 **********************************************************************/
static std::vector<Pothos::PluginPath> loadConfFile(const std::string &path)
{
    std::vector<Pothos::PluginPath> entries;
    poco_debug_f1(confLoaderLogger(), "loading %s", path);

    //parse the configuration file with the INI parser
    Poco::AutoPtr<Poco::Util::PropertyFileConfiguration> conf(new Poco::Util::PropertyFileConfiguration(path));

    //iterate through each section
    Poco::Util::AbstractConfiguration::Keys keys; conf->keys(keys);

    //create a mapping of the config data
    std::map<std::string, std::string> configMap;
    for (const auto &key : keys) configMap[key] = conf->getString(key);

    //and other config file parameters
    configMap["confFilePath"] = path;

    //handle the loader
    POTHOS_EXCEPTION_TRY
    {
        //get the loader
        if (not conf->hasProperty("loader")) throw Pothos::Exception(
            Poco::format("%s does not specify a loader", path));
        const std::string loader(conf->getString("loader"));
        const auto loaderPath = Pothos::PluginPath("/framework/conf_loader").join(loader);
        if (not Pothos::PluginRegistry::exists(loaderPath)) throw Pothos::Exception(
            Poco::format("%s loader %s does not exist", path, loader));

        //call the loader
        const auto plugin = Pothos::PluginRegistry::get(loaderPath);
        const auto &loaderFcn = plugin.getObject().extract<Pothos::Callable>();
        entries = loaderFcn.call<std::vector<Pothos::PluginPath>>(configMap);
    }
    POTHOS_EXCEPTION_CATCH (const Pothos::Exception &ex)
    {
        //log an error here, but do not re-throw when a particular loader fails
        //we must return successfully all loaded entries so they can be unloaded later
        confLoaderLogger().error("%s\n\t%s", path, ex.message());
    }

    return entries;
}

/***********************************************************************
 * Traverse a path for configuration files
 **********************************************************************/
static std::vector<Poco::Path> getConfFilePaths(const Poco::Path &path)
{
    poco_debug_f1(confLoaderLogger(), "traversing %s", path.toString());

    std::vector<Poco::Path> paths;

    const Poco::File file(path);
    if (not file.exists()) return paths;
    else if (file.isFile() and (path.getExtension() == "conf"))
    {
        paths.push_back(Pothos_FileRealPath(path.toString()));
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

/***********************************************************************
 * Entry point: traversal and loading of configuration files
 **********************************************************************/
std::vector<Pothos::PluginPath> Pothos_ConfLoader_loadConfFiles(void)
{
    //the default search path
    std::vector<Poco::Path> searchPaths;
    Poco::Path confPath = Pothos::System::getRootPath();
    confPath.append("share");
    confPath.append("Pothos");
    confPath.append("modules");
    searchPaths.push_back(confPath);

    //support /usr/local module installs when the install prefix is /usr
    if (Pothos::System::getRootPath() == "/usr")
    {
        searchPaths.push_back("/usr/local/share/Pothos/modules");
    }

    //the user's home config path
    confPath = Pothos::System::getUserConfigPath();
    confPath.append("modules");
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
    std::vector<std::future<std::vector<Pothos::PluginPath>>> futures;
    for (const auto &searchPath : searchPaths)
    {
        for (const auto &path : getConfFilePaths(searchPath.absolute()))
        {
            futures.push_back(std::async(std::launch::async, &loadConfFile, path.toString()));
        }
    }

    //wait for completion of future module load
    std::vector<Pothos::PluginPath> entries;
    for (auto &future : futures)
    {
        POTHOS_EXCEPTION_TRY
        {
            auto subEntries = future.get();
            entries.insert(entries.end(), subEntries.begin(), subEntries.end());
        }
        POTHOS_EXCEPTION_CATCH (const Pothos::Exception &ex)
        {
            poco_error(confLoaderLogger(), ex.displayText());
        }
    }
    return entries;
}
