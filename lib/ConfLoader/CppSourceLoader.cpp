// Copyright (c) 2016-2016 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Plugin.hpp>
#include <Pothos/Util/Compiler.hpp>
#include <Poco/Path.h>
#include <Poco/File.h>
#include <Poco/StringTokenizer.h>
#include <map>

//TODO finish compiler stuff

/***********************************************************************
 * Compile and load blocks built from C++ sources
 **********************************************************************/
static std::vector<Pothos::PluginPath> cppSourceLoader(const std::map<std::string, std::string> &config)
{
    std::vector<Pothos::PluginPath> entries;

    //config file path set by caller
    const auto confFilePathIt = config.find("confFilePath");
    if (confFilePathIt == config.end() or confFilePathIt->second.empty())
        throw Pothos::Exception("missing confFilePath");
    const auto rootDir = Poco::Path(confFilePathIt->second).makeParent();

    //load the compiler args
    auto compilerArgs = Pothos::Util::CompilerArgs::defaultDevEnv();
    const auto tokOptions = Poco::StringTokenizer::TOK_TRIM | Poco::StringTokenizer::TOK_TRIM;

    //load the includes: allow CSV format, make absolute to the config dir
    const auto includesIt = config.find("includes");
    if (includesIt != config.end()) for (const auto &include :
        Poco::StringTokenizer(includesIt->second, ",", tokOptions))
    {
        const auto absPath = Poco::Path(include).makeAbsolute(rootDir);
        compilerArgs.includes.push_back(absPath.toString());
    }

    //load the libraries: allow CSV format, make absolute to the config dir
    const auto librariesIt = config.find("libraries");
    if (librariesIt != config.end()) for (const auto &library :
        Poco::StringTokenizer(librariesIt->second, ",", tokOptions))
    {
        const auto absPath = Poco::Path(library).makeAbsolute(rootDir);
        compilerArgs.libraries.push_back(absPath.toString());
    }

    //load the sources: allow CSV format, make absolute to the config dir
    const auto sourcesIt = config.find("sources");
    if (sourcesIt != config.end()) for (const auto &source :
        Poco::StringTokenizer(sourcesIt->second, ",", tokOptions))
    {
        const auto absPath = Poco::Path(source).makeAbsolute(rootDir);
        compilerArgs.sources.push_back(absPath.toString());
    }

    //load the flags: allow CSV format (TODO handle escaped commas?)
    const auto flagsIt = config.find("flags");
    if (flagsIt != config.end()) for (const auto &flag :
        Poco::StringTokenizer(flagsIt->second, ",", tokOptions))
    {
        compilerArgs.flags.push_back(flag);
    }

    return entries;
}

/***********************************************************************
 * loader registration
 **********************************************************************/
pothos_static_block(pothosFrameworkRegisterCppSourceLoader)
{
    Pothos::PluginRegistry::addCall("/framework/conf_loader/CppSource", &cppSourceLoader);
}
