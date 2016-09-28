// Copyright (c) 2016-2016 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Plugin.hpp>
#include <Pothos/System.hpp>
#include <Pothos/Util/Compiler.hpp>
#include <Poco/Logger.h>
#include <Poco/Path.h>
#include <Poco/File.h>
#include <Poco/Pipe.h>
#include <Poco/PipeStream.h>
#include <Poco/StringTokenizer.h>
#include <Poco/SharedLibrary.h>
#include <Poco/Process.h>
#include <map>

static Poco::Logger &sourceLoaderLogger(void)
{
    static Poco::Logger &logger(Poco::Logger::get("Pothos.SourceLoader"));
    return logger;
}

/***********************************************************************
 * Helper to manage recompile
 **********************************************************************/
static void compilationHelper(
    const std::string &target, const Poco::File &outFile,
    Pothos::Util::CompilerArgs &compilerArgs)
{
    //check if we need to recompile
    bool recompile = false;
    if (outFile.exists())
    {
        const auto lastTimeCompiled = outFile.getLastModified();
        const auto devLib = Pothos::System::getPothosDevLibraryPath();
        if (Poco::File(devLib).getLastModified() > lastTimeCompiled) recompile = true;
        for (const auto &source : compilerArgs.sources)
        {
            if (Poco::File(source).getLastModified() > lastTimeCompiled) recompile = true;
        }
    }
    else recompile = true;
    if (not recompile) return;

    //compiler instance
    const auto compiler = Pothos::Util::Compiler::make();

    //generate JSON block descriptions
    Poco::Process::Args args;
    args.push_back("--doc-parse");
    for (const auto &source : compilerArgs.sources)
    {
        args.push_back(source);
    }
    const auto blockDescs = compiler->createTempFile(".cpp");
    args.push_back("--output");
    args.push_back(blockDescs);
    compilerArgs.sources.push_back(blockDescs);

    sourceLoaderLogger().information("Parse sources for %s...", target);
    Poco::Pipe inPipe, outPipe;
    Poco::Process::Env env;
    Poco::ProcessHandle ph(Poco::Process::launch(
        Pothos::System::getPothosUtilExecutablePath(),
        args, &inPipe, &outPipe, &outPipe, env));

    //handle error case
    if (ph.wait() != 0)
    {
        Poco::PipeInputStream errStream(outPipe);
        const std::string errMsgBuff = std::string(
            std::istreambuf_iterator<char>(errStream),
            std::istreambuf_iterator<char>());
        throw Pothos::Exception("PothosUtil --doc-parse", errMsgBuff);
    }

    //compile
    sourceLoaderLogger().information("Compile sources for %s...", target);
    const auto tmpOutput = compiler->compileCppModule(compilerArgs);
    Poco::File(tmpOutput).moveTo(outFile.path());
    sourceLoaderLogger().information("Wrote %s", outFile.path());
}

/***********************************************************************
 * Compile and load blocks built from C++ sources
 **********************************************************************/
static std::vector<Pothos::PluginPath> JITCompilerLoader(const std::map<std::string, std::string> &config)
{
    std::vector<Pothos::PluginPath> entries;

    //config file path set by caller
    const auto confFilePathIt = config.find("confFilePath");
    if (confFilePathIt == config.end() or confFilePathIt->second.empty())
        throw Pothos::Exception("missing confFilePath");
    const auto rootDir = Poco::Path(confFilePathIt->second).makeParent();

    //config section set by caller
    const auto confFileSectionIt = config.find("confFileSection");
    if (confFileSectionIt == config.end() or confFileSectionIt->second.empty())
        throw Pothos::Exception("missing confFileSection");
    const auto &target = confFileSectionIt->second;

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

    //determine output file
    Poco::Path outPath(Pothos::System::getUserDataPath());
    outPath.append("blocks");
    Poco::File(outPath).createDirectories();
    outPath.append(target + Poco::SharedLibrary::suffix());

    //compile if changed
    compilationHelper(target, outPath, compilerArgs);

    //load the module
    Pothos::PluginModule pluginModule(outPath.toString());

    //store the handle in the registry
    const auto pluginPath = Pothos::PluginPath("/framework/conf_loader/jit_compiler/blocks").join(target);
    Pothos::PluginRegistry::add(pluginPath, pluginModule);
    entries.push_back(pluginPath);
    return entries;
}

/***********************************************************************
 * loader registration
 **********************************************************************/
pothos_static_block(pothosFrameworkRegisterCppSourceLoader)
{
    Pothos::PluginRegistry::addCall("/framework/conf_loader/jit_compiler", &JITCompilerLoader);
}
