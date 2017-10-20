// Copyright (c) 2013-2017 Josh Blum
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
#include <mutex>
#include <cctype>

//! The path used to cache the safe loads
static std::string getModuleLoaderCachePath(void)
{
    Poco::Path path(Pothos::System::getUserConfigPath());
    path.append("ModuleLoader.cache");
    return path.toString();
}

/***********************************************************************
 * named mutex for cache protection
 **********************************************************************/
struct LoaderCacheFileLock : public Pothos::Util::FileLock
{
    LoaderCacheFileLock(void):
        Pothos::Util::FileLock(getModuleLoaderCachePath()+".lock")
    {}
};

static Pothos::Util::FileLock &getLoaderFileLock(void)
{
    static LoaderCacheFileLock lock;
    return lock;
}

static std::mutex &getLoaderMutex(void)
{
    static std::mutex mutex;
    return mutex;
}

/***********************************************************************
 * calls to interface with file cache
 **********************************************************************/
//! Get the string representation of a file's modification time
static std::string getLastModifiedTimeStr(const std::string &path)
{
    return std::to_string(Poco::File(path).getLastModified().epochMicroseconds());
}

//! Escape PropertyFile keys, problem with slashes
static std::string escape(const std::string &in)
{
    std::string out;
    for (const auto ch : in)
    {
        if (std::isalnum(ch)) out.push_back(ch);
        else out.push_back('_');
    }
    return out;
}

//! Was a previous safe-load of this module successful?
static bool previousLoadWasSuccessful(const std::string &modulePath)
{
    std::lock_guard<std::mutex> mutexLock(getLoaderMutex());
    std::lock_guard<Pothos::Util::FileLock> fileLock(getLoaderFileLock());
    Poco::AutoPtr<Poco::Util::PropertyFileConfiguration> cache(new Poco::Util::PropertyFileConfiguration());
    try {cache->load(getModuleLoaderCachePath());} catch(...){}

    try
    {
        auto libTime = getLastModifiedTimeStr(Pothos::System::getPothosRuntimeLibraryPath());
        auto modTime = getLastModifiedTimeStr(modulePath);
        auto cachedLibTime = cache->getString(escape(Pothos::System::getPothosRuntimeLibraryPath()));
        auto cachedModTime = cache->getString(escape(modulePath));
        return (libTime == cachedLibTime) and (modTime == cachedModTime);
    }
    catch (const Poco::NotFoundException &){}
    return false;
}

//! Mark that the safe load of this module was successful
static void markCurrentLoadSuccessful(const std::string &modulePath)
{
    std::lock_guard<std::mutex> mutexLock(getLoaderMutex());
    std::lock_guard<Pothos::Util::FileLock> fileLock(getLoaderFileLock());
    Poco::AutoPtr<Poco::Util::PropertyFileConfiguration> cache(new Poco::Util::PropertyFileConfiguration());
    try {cache->load(getModuleLoaderCachePath());} catch(...){}

    auto libTime = getLastModifiedTimeStr(Pothos::System::getPothosRuntimeLibraryPath());
    auto modTime = getLastModifiedTimeStr(modulePath);
    cache->setString(escape(Pothos::System::getPothosRuntimeLibraryPath()), libTime);
    cache->setString(escape(modulePath), modTime);
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
