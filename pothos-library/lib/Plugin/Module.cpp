// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/System/Paths.hpp>
#include <Pothos/Plugin/Module.hpp>
#include <Pothos/Plugin/Exception.hpp>
#include <Pothos/Object.hpp> //pulls in full Object implementation
#include <Poco/SharedLibrary.h>
#include <Poco/Logger.h>
#include <Poco/Format.h>
#include <mutex>
#include <Poco/SingletonHolder.h>
#include <Poco/Process.h>
#include <Poco/Pipe.h>

/***********************************************************************
 * Locking for plugin attach helper
 **********************************************************************/
void registrySetActiveModuleLoading(const Pothos::PluginModule &module);

static std::mutex &getModuleMutex(void)
{
    static Poco::SingletonHolder<std::mutex> sh;
    return *sh.get();
}

/***********************************************************************
 * Private implementation -- unloads library automatically
 **********************************************************************/
struct Pothos::PluginModule::Impl
{
    ~Impl(void)
    {
        if (sharedLibrary.isLoaded())
        {
            poco_information(Poco::Logger::get("Pothos.PluginModule.unload"), sharedLibrary.getPath());
            sharedLibrary.unload();
        }
    }

    Poco::SharedLibrary sharedLibrary;
    std::string path;
};

/***********************************************************************
 * Module implementation
 **********************************************************************/
Pothos::PluginModule::PluginModule(void)
{
    return;
}

Pothos::PluginModule::PluginModule(const std::string &path):
    _impl(new Impl())
{
    _impl->path = path;
    poco_information(Poco::Logger::get("Pothos.PluginModule.load"), path);
    try
    {
        std::lock_guard<std::mutex> lock(getModuleMutex());
        registrySetActiveModuleLoading(*this);
        _impl->sharedLibrary.load(path);
        registrySetActiveModuleLoading(PluginModule());
    }
    catch(const Poco::LibraryLoadException &ex)
    {
        throw PluginModuleError("Pothos::PluginModule("+path+")", ex.displayText());
    }
}

Pothos::PluginModule Pothos::PluginModule::safeLoad(const std::string &path)
{
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
    return PluginModule(path);
}

std::string Pothos::PluginModule::getFilePath(void) const
{
    if (not _impl) return "";
    return _impl->path;
}

#include <Pothos/Managed.hpp>

static auto managedPluginModule = Pothos::ManagedClass()
    .registerConstructor<Pothos::PluginModule>()
    .registerConstructor<Pothos::PluginModule, const std::string &>()
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::PluginModule, getFilePath))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::PluginModule, getPluginPaths))
    .commit("Pothos/PluginModule");
