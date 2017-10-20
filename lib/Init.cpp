// Copyright (c) 2013-2017 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Init.hpp>
#include <Pothos/Exception.hpp>
#include <Pothos/System/Paths.hpp>
#include <Pothos/Plugin.hpp>
#include <Poco/Path.h>
#include <Poco/File.h>
#include <Poco/Format.h>

//from lib/Framework/ConfLoader.cpp
std::vector<Pothos::PluginPath> Pothos_ConfLoader_loadConfFiles(void);

/***********************************************************************
 * Singleton for initialization once per process
 **********************************************************************/
namespace Pothos {

    class InitSingleton
    {
    public:
        InitSingleton(void);
        static InitSingleton &instance(void);
        void load(void);
        void unload(void);

    private:
        std::vector<Pothos::PluginModule> modules;
        std::vector<Pothos::PluginPath> confLoadedPaths;
    };

} //namespace Pothos

Pothos::InitSingleton &Pothos::InitSingleton::instance(void)
{
    static Pothos::InitSingleton inst;
    return inst;
}

void Pothos::InitSingleton::load(void)
{
    if (not modules.empty()) return;
    modules = PluginLoader::loadModules();
    confLoadedPaths = Pothos_ConfLoader_loadConfFiles();
}

void Pothos::InitSingleton::unload(void)
{
    for (const auto &path : confLoadedPaths)
    {
        Pothos::PluginRegistry::remove(path);
    }
    confLoadedPaths.clear();
    modules.clear();
}

/***********************************************************************
 * Actual init implementation
 **********************************************************************/
Pothos::InitSingleton::InitSingleton(void)
{
    //check the root
    const Poco::File rootPath(System::getRootPath());
    if (not rootPath.exists() or not rootPath.isDirectory()) throw Exception(
        "Pothos::init()", Poco::format(
        "Pothos installation FAIL - \"%s\" does not exist",
        System::getRootPath()
    ));

    //check for the util executable
    const Poco::File utilExecutablePath(System::getPothosUtilExecutablePath());
    if (not utilExecutablePath.exists()) throw Exception(
        "Pothos::init()", Poco::format(
        "Pothos installation FAIL - \"%s\" does not exist",
        System::getPothosUtilExecutablePath()
    ));
    if (not utilExecutablePath.canExecute()) throw Exception(
        "Pothos::init()", Poco::format(
        "Pothos installation FAIL - \"%s\" not executable",
        System::getPothosUtilExecutablePath()
    ));

    //check for the runtime library
    const Poco::File runtimeLibraryPath(System::getPothosRuntimeLibraryPath());
    if (not runtimeLibraryPath.exists() or not runtimeLibraryPath.isFile()) throw Exception(
        "Pothos::init()", Poco::format(
        "Pothos installation FAIL - \"%s\" does not exist",
        System::getPothosRuntimeLibraryPath()
    ));

    //check the user data directory
    Poco::File userDataPath(System::getUserDataPath());
    if (not userDataPath.exists()) userDataPath.createDirectories();
    if (not userDataPath.exists() or not userDataPath.canWrite()) throw Exception(
        "Pothos::init()", Poco::format(
        "Pothos user directory FAIL - \"%s\" permissions error",
        System::getUserDataPath()
    ));

    //check the user config directory
    Poco::File userConfigPath(System::getUserConfigPath());
    if (not userConfigPath.exists()) userConfigPath.createDirectories();
    if (not userConfigPath.exists() or not userConfigPath.canWrite()) throw Exception(
        "Pothos::init()", Poco::format(
        "Pothos user directory FAIL - \"%s\" permissions error",
        System::getUserConfigPath()
    ));

    //check the development environment (includes)
    Poco::File devIncludePath(System::getPothosDevIncludePath());
    if (not devIncludePath.exists()) throw Exception(
        "Pothos::init()", Poco::format(
        "Pothos development include directory FAIL - \"%s\" does not exist",
        System::getPothosDevIncludePath()
    ));

    //check the development environment (libraries)
    Poco::File devLibraryPath(System::getPothosDevLibraryPath());
    if (not devLibraryPath.exists()) throw Exception(
        "Pothos::init()", Poco::format(
        "Pothos development library directory FAIL - \"%s\" does not exist",
        System::getPothosDevLibraryPath()
    ));
}

void Pothos::init(void)
{
    Pothos::InitSingleton::instance().load();
}

void Pothos::deinit(void)
{
    Pothos::InitSingleton::instance().unload();
}

Pothos::ScopedInit::ScopedInit(void)
{
    Pothos::init();
}

Pothos::ScopedInit::~ScopedInit(void)
{
    Pothos::deinit();
}
