// Copyright (c) 2016-2016 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Plugin.hpp>
#include <Pothos/System.hpp>
#include <Pothos/Util/Compiler.hpp>
#include <Pothos/Util/FileLock.hpp>
#include <Poco/Logger.h>
#include <Poco/Path.h>
#include <Poco/File.h>
#include <Poco/Pipe.h>
#include <Poco/PipeStream.h>
#include <Poco/StringTokenizer.h>
#include <Poco/SharedLibrary.h>
#include <Poco/Process.h>
#include <memory>
#include <mutex>
#include <map>

static Poco::Logger &sourceLoaderLogger(void)
{
    static Poco::Logger &logger(Poco::Logger::get("Pothos.JITCompiler"));
    return logger;
}

struct RegistryJITResult
{
    RegistryJITResult(void):
        loaded(false){}

    std::mutex mutex;
    bool loaded;
    Pothos::PluginModule pluginModule;
    std::vector<Pothos::PluginPath> factories;
};

std::vector<Pothos::PluginPath> blockDescParser(std::istream &is, std::vector<Pothos::PluginPath> &blockPaths);

/***********************************************************************
 * Helper to manage recompile
 **********************************************************************/
static void compilationHelper(
    const std::string &target, const Poco::File &outFile,
    const Pothos::Util::CompilerArgs &compilerArgs)
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

    //compile
    sourceLoaderLogger().information("Compile sources for %s...", target);
    const auto tmpOutput = compiler->compileCppModule(compilerArgs);
    Poco::File(tmpOutput).moveTo(outFile.path());
    sourceLoaderLogger().information("Wrote %s", outFile.path());
}

/***********************************************************************
 * The JIT compiler factory compiles sources on demand
 * and replaces its plugin registration with the compiled on
 **********************************************************************/
static Pothos::Object opaqueJITCompilerFactory(
    std::shared_ptr<RegistryJITResult> handle,
    const std::string &target, const Poco::Path &outPath,
    const Pothos::Util::CompilerArgs &compilerArgs,
    const Pothos::PluginPath &pluginPath,
    const Pothos::Object *args,
    const size_t numArgs)
{
    //local mutex lock for the registry
    std::lock_guard<std::mutex> lock(handle->mutex);

    //file lock for compilation atomicity across processes
    Pothos::Util::FileLock outputFileLock(outPath.toString());
    std::lock_guard<Pothos::Util::FileLock> fileLock(outputFileLock);

    //compile if changed
    compilationHelper(target, outPath, compilerArgs);

    //load the module, only once for all registered entries
    if (not handle->loaded)
    {
        //remove all registrations before loading
        for (const auto &pluginPath : handle->factories)
        {
            Pothos::PluginRegistry::remove(pluginPath);
        }

        handle->pluginModule = Pothos::PluginModule(outPath.toString());
        handle->loaded = true;
    }

    //the actual function from the compiled module
    const auto plugin = Pothos::PluginRegistry::get(pluginPath);
    const auto &call = plugin.getObject().extract<Pothos::Callable>();
    return call.opaqueCall(args, numArgs);
}

/***********************************************************************
 * Register factory functions that will compile the source
 **********************************************************************/
static std::vector<Pothos::PluginPath> JITCompilerLoader(const std::map<std::string, std::string> &config)
{
    std::vector<Pothos::PluginPath> entries;

    //create handle to hold shared compilation result
    std::shared_ptr<RegistryJITResult> handle(new RegistryJITResult());

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

    //load the factories: use this when providing no block description
    const auto factoriesIt = config.find("factories");
    if (factoriesIt != config.end()) for (const auto &factory :
        Poco::StringTokenizer(factoriesIt->second, ",", tokOptions))
    {
        handle->factories.push_back(Pothos::PluginPath("/blocks").join(factory.substr(1)));
    }

    //determine output file
    Poco::Path outPath(Pothos::System::getUserDataPath());
    outPath.append("blocks");
    Poco::File(outPath).createDirectories();
    outPath.append(target + Poco::SharedLibrary::suffix());

    //generate JSON block descriptions
    Poco::Process::Args args;
    args.push_back("--doc-parse");
    for (const auto &source : compilerArgs.sources)
    {
        args.push_back(source);
    }
    args.push_back("--success-code");
    args.push_back("200");

    Poco::Pipe inPipe, outPipe;
    Poco::Process::Env env;
    Poco::ProcessHandle ph(Poco::Process::launch(
        Pothos::System::getPothosUtilExecutablePath(),
        args, &inPipe, &outPipe, &outPipe, env));
    Poco::PipeInputStream outStream(outPipe);

    //handle error case
    if (ph.wait() != 200)
    {
        const std::string errMsgBuff = std::string(
            std::istreambuf_iterator<char>(outStream),
            std::istreambuf_iterator<char>());
        throw Pothos::Exception("PothosUtil --doc-parse", errMsgBuff);
    }

    //parse the json, store block paths in handle, and store doc paths
    for (const auto &docPath : blockDescParser(outStream, handle->factories))
    {
        entries.push_back(docPath);
    }

    //register for all factory paths
    for (const auto &pluginPath : handle->factories)
    {
        const auto factory = Pothos::Callable(&opaqueJITCompilerFactory)
            .bind(handle, 0)
            .bind(target, 1)
            .bind(outPath, 2)
            .bind(compilerArgs, 3)
            .bind(pluginPath, 4);
        Pothos::PluginRegistry::addCall(pluginPath, factory);
        entries.push_back(pluginPath);
    }

    //store the handle in the registry
    const auto pluginPath = Pothos::PluginPath("/framework/conf_loader/jit_compiler/blocks").join(target);
    Pothos::PluginRegistry::add(pluginPath, handle);
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
