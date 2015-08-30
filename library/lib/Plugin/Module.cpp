// Copyright (c) 2013-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/System/Paths.hpp>
#include <Pothos/Plugin/Module.hpp>
#include <Pothos/Plugin/Exception.hpp>
#include <Pothos/Object.hpp> //pulls in full Object implementation
#include <Poco/SharedLibrary.h>
#include <Poco/Platform.h>
#include <Poco/Logger.h>
#include <Poco/Format.h>
#include <mutex>
#include <Poco/SingletonHolder.h>

/***********************************************************************
 * Disabler for windows error messages
 **********************************************************************/
#if POCO_OS == POCO_OS_WINDOWS_NT

#include <windows.h>

BOOL DL_SetThreadErrorMode(DWORD dwNewMode, LPDWORD lpOldMode);

struct ErrorMessageDisableGuard
{
    ErrorMessageDisableGuard(void)
    {
        DL_SetThreadErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX, &oldMode);
    }
    ~ErrorMessageDisableGuard(void)
    {
        DL_SetThreadErrorMode(oldMode, nullptr);
    }
    DWORD oldMode;
};

#else
struct ErrorMessageDisableGuard
{
    ErrorMessageDisableGuard(void){}
    ~ErrorMessageDisableGuard(void){}
};
#endif

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
        ErrorMessageDisableGuard emdg;
        _impl->sharedLibrary.load(path);
        registrySetActiveModuleLoading(PluginModule());
    }
    catch(const Poco::LibraryLoadException &ex)
    {
        throw PluginModuleError("Pothos::PluginModule("+path+")", ex.displayText());
    }
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
