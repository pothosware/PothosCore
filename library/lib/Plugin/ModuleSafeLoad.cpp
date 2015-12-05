// Copyright (c) 2013-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/System/Paths.hpp>
#include <Pothos/Plugin/Module.hpp>
#include <Pothos/Plugin/Exception.hpp>
#include <Pothos/Util/FileLock.hpp>
#include <Poco/Process.h>
#include <Poco/Pipe.h>
#include <Poco/File.h>
#include <Poco/Path.h>
#include <Poco/AutoPtr.h>
#include <Poco/Util/PropertyFileConfiguration.h>
#include <Poco/SingletonHolder.h>
#include <mutex>

/***********************************************************************
 * named mutex for cache protection
 **********************************************************************/
struct LoaderCacheFileLock : public Pothos::Util::FileLock
{
    LoaderCacheFileLock(void):
        Pothos::Util::FileLock(getModuleLoaderLockPath())
    {}

    static std::string getModuleLoaderLockPath(void)
    {
        Poco::Path path(Pothos::System::getUserConfigPath());
        path.append("ModuleLoader.lock");
        return path.toString();
    }
};

static Pothos::Util::FileLock &getLoaderFileLock(void)
{
    static Poco::SingletonHolder<LoaderCacheFileLock> sh;
    return *sh.get();
}

static std::mutex &getLoaderMutex(void)
{
    static Poco::SingletonHolder<std::mutex> sh;
    return *sh.get();
}

/***********************************************************************
 * calls to interface with file cache
 **********************************************************************/
//! The path used to cache the safe loads
static std::string getModuleLoaderCachePath(void)
{
    Poco::Path path(Pothos::System::getUserConfigPath());
    path.append("ModuleLoader.cache");
    return path.toString();
}

//! Get the string representation of a file's modification time
static std::string getLastModifiedTimeStr(const std::string &path)
{
    return std::to_string(Poco::File(path).getLastModified().epochMicroseconds());
}

//! Was a previous safe-load of this module successful?
static bool previousLoadWasSuccessful(const std::string &modulePath)
{
    std::lock_guard<std::mutex> mutexLock(getLoaderMutex());
    std::lock_guard<Pothos::Util::FileLock> lock(getLoaderFileLock());
    Poco::AutoPtr<Poco::Util::PropertyFileConfiguration> cache(new Poco::Util::PropertyFileConfiguration());
    try {cache->load(getModuleLoaderCachePath());} catch(...){}

    try
    {
        auto libTime = getLastModifiedTimeStr(Pothos::System::getPothosRuntimeLibraryPath());
        auto modTime = getLastModifiedTimeStr(modulePath);
        auto cachedLibTime = cache->getString(Pothos::System::getPothosRuntimeLibraryPath());
        auto cachedModTime = cache->getString(modulePath);
        return (libTime == cachedLibTime) and (modTime == cachedModTime);
    }
    catch (const Poco::NotFoundException &){}
    return false;
}

//! Mark that the safe load of this module was successful
static void markCurrentLoadSuccessful(const std::string &modulePath)
{
    std::lock_guard<std::mutex> mutexLock(getLoaderMutex());
    std::lock_guard<Pothos::Util::FileLock> lock(getLoaderFileLock());
    Poco::AutoPtr<Poco::Util::PropertyFileConfiguration> cache(new Poco::Util::PropertyFileConfiguration());
    try {cache->load(getModuleLoaderCachePath());} catch(...){}

    auto libTime = getLastModifiedTimeStr(Pothos::System::getPothosRuntimeLibraryPath());
    auto modTime = getLastModifiedTimeStr(modulePath);
    cache->setString(Pothos::System::getPothosRuntimeLibraryPath(), libTime);
    cache->setString(modulePath, modTime);
    try {cache->save(getModuleLoaderCachePath());} catch(...){}
}

/***********************************************************************
 * module safe load implementation
 **********************************************************************/
Pothos::PluginModule Pothos::PluginModule::safeLoad(const std::string &path)
{
    if (previousLoadWasSuccessful(path)) return PluginModule(path);

    const int success = 200;

    //create args
    Poco::Process::Args args;
    args.push_back("--load-module");
    args.push_back("\""+path+"\""); //add quotes for paths with spaces
    args.push_back("--success-code");
    args.push_back(std::to_string(success));

    //launch
    Poco::Pipe outPipe, errPipe;
    Poco::Process::Env env;
    Poco::ProcessHandle ph(Poco::Process::launch(
        Pothos::System::getPothosUtilExecutablePath(),
        args, nullptr, &outPipe, &errPipe, env));

    //close pipes to not overfill and backup
    outPipe.close();
    errPipe.close();

    //wait, check error condition, and throw
    if (ph.wait() != success)
    {
        throw Pothos::PluginModuleError("Pothos::PluginModule("+path+")", "failed safe load");
    }

    //it was safe, load into this process now
    markCurrentLoadSuccessful(path);
    return PluginModule(path);
}
