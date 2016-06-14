// Copyright (c) 2013-2016 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/System/Paths.hpp>
#include <Pothos/Plugin/Module.hpp>
#include <Pothos/Plugin/Registry.hpp>
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

std::vector<std::string> getPluginPaths(const Pothos::PluginModule &module);

static std::mutex &getModuleMutex(void)
{
    static Poco::SingletonHolder<std::mutex> sh;
    return *sh.get();
}

/***********************************************************************
 * Shared implementation for module data
 **********************************************************************/
struct Pothos::PluginModule::Impl
{
    Poco::SharedLibrary sharedLibrary;
    std::string path;
    std::vector<std::string> pluginPaths;
};

/***********************************************************************
 * Module implementation
 **********************************************************************/
const Pothos::PluginModule &Pothos::PluginModule::null(void)
{
    static PluginModule nullModule;
    return nullModule;
}

Pothos::PluginModule::PluginModule(void)
{
    return;
}

Pothos::PluginModule::PluginModule(const std::string &path):
    _impl(new Impl())
{
    _impl->path = path;
    poco_debug(Poco::Logger::get("Pothos.PluginModule.load"), path);
    try
    {
        std::lock_guard<std::mutex> lock(getModuleMutex());
        registrySetActiveModuleLoading(*this);
        ErrorMessageDisableGuard emdg;
        _impl->sharedLibrary.load(path);
        registrySetActiveModuleLoading(PluginModule());
        _impl->pluginPaths = ::getPluginPaths(*this);
    }
    catch(const Poco::LibraryLoadException &ex)
    {
        throw PluginModuleError("Pothos::PluginModule("+path+")", ex.displayText());
    }
}

Pothos::PluginModule::~PluginModule(void)
{
    if (not _impl) return; //no private data
    if (not _impl.unique()) return; //this is not the last copy
    if (not _impl->sharedLibrary.isLoaded()) return; //module not loaded

    poco_debug(Poco::Logger::get("Pothos.PluginModule.unload"), _impl->sharedLibrary.getPath());
    for (const auto &pluginPath : this->getPluginPaths())
    {
        PluginRegistry::remove(pluginPath);
    }
    _impl->sharedLibrary.unload();
}

std::string Pothos::PluginModule::getFilePath(void) const
{
    if (not _impl) return "";
    return _impl->path;
}

const std::vector<std::string> &Pothos::PluginModule::getPluginPaths(void) const
{
    return _impl->pluginPaths;
}

#include <Pothos/Managed.hpp>

static auto managedPluginModule = Pothos::ManagedClass()
    .registerConstructor<Pothos::PluginModule>()
    .registerConstructor<Pothos::PluginModule, const std::string &>()
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::PluginModule, getFilePath))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::PluginModule, getPluginPaths))
    .commit("Pothos/PluginModule");
