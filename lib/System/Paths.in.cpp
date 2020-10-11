// Copyright (c) 2013-2016 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/System/Paths.hpp>
#include <Pothos/System/Version.hpp>
#include <Pothos/System/Exception.hpp>
#include <Poco/Environment.h>
#include <Poco/Path.h>
#include <Poco/Foundation.h>
#include <Poco/StringTokenizer.h>
#include <algorithm>
#include <iterator>

std::string Pothos::System::getRootPath(void)
{
    if (Poco::Environment::has("@POTHOS_ROOT_ENV@"))
        return Poco::Path(Poco::Environment::get("@POTHOS_ROOT_ENV@")).absolute().toString();

    // Get the path to the current dynamic linked library.
    // The path to this library can be used to determine
    // the installation root without prior knowledge.
    #if defined(POCO_OS_FAMILY_WINDOWS)
    char path[MAX_PATH];
    HMODULE hm = NULL;
    if (GetModuleHandleExA(
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
        GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        (LPCSTR) &Pothos::System::getRootPath, &hm))
    {
        const DWORD size = GetModuleFileNameA(hm, path, sizeof(path));
        if (size != 0)
        {
            const Poco::Path libPath(std::string(path, size), Poco::Path::PATH_WINDOWS);
            return libPath.parent().popDirectory().toString();
        }
    }
    #endif

    //assume that the root path is always in UNIX style
    const Poco::Path configPrefix = Poco::Path("@POTHOS_ROOT@", Poco::Path::PATH_UNIX);
    return configPrefix.absolute().toString();
}

std::string Pothos::System::getDataPath(void)
{
    Poco::Path dataPath(getRootPath());
    dataPath.append("@CMAKE_INSTALL_DATADIR@");
    dataPath.append("Pothos");
    return dataPath.absolute().toString();
}

std::string Pothos::System::getUserDataPath(void)
{
    Poco::Path dataPath;
    if (Poco::Environment::has("XDG_DATA_HOME"))
    {
        dataPath = Poco::Path(Poco::Environment::get("XDG_DATA_HOME"));
    }
    else if (Poco::Environment::has("@APPDATA_ENV@"))
    {
        dataPath = Poco::Path(Poco::Environment::get("@APPDATA_ENV@"));
    }
    else
    {
        dataPath = Poco::Path::home();
        dataPath.append(".local");
        dataPath.append("@CMAKE_INSTALL_DATADIR@");
    }
    dataPath.append("Pothos");
    return dataPath.absolute().toString();
}

std::string Pothos::System::getUserConfigPath(void)
{
    Poco::Path configPath;
    if (Poco::Environment::has("XDG_CONFIG_HOME"))
    {
        configPath = Poco::Path(Poco::Environment::get("XDG_CONFIG_HOME"));
    }
    else if (Poco::Environment::has("@APPDATA_ENV@"))
    {
        configPath = Poco::Path(Poco::Environment::get("@APPDATA_ENV@"));
    }
    else
    {
        configPath = Poco::Path::home();
        configPath.append(".config");
    }
    configPath.append("Pothos");
    return configPath.absolute().toString();
}

std::string Pothos::System::getPothosUtilExecutablePath(void)
{
    Poco::Path utilPath(Pothos::System::getRootPath());
    utilPath.append("@CMAKE_INSTALL_BINDIR@");
    utilPath.append("PothosUtil");
    #if defined(POCO_OS_FAMILY_WINDOWS)
    utilPath.setExtension("exe");
    #endif

    return utilPath.absolute().toString();
}

std::string Pothos::System::getPothosRuntimeLibraryPath(void)
{
    Poco::Path dllPath(Pothos::System::getRootPath());
    #if defined(POCO_OS_FAMILY_WINDOWS)
    dllPath.append("@CMAKE_INSTALL_BINDIR@");
    #elif defined(POCO_OS_FAMILY_UNIX)
    dllPath.append("@CMAKE_INSTALL_LIBDIR@");
    #endif
    dllPath.append(POTHOS_LIBRARY_NAME);

    return dllPath.absolute().toString();
}

std::string Pothos::System::getPothosDevIncludePath(void)
{
    Poco::Path incPath(Pothos::System::getRootPath());
    incPath.append("@CMAKE_INSTALL_INCLUDEDIR@");
    return incPath.absolute().toString();
}

std::string Pothos::System::getPothosDevLibraryPath(void)
{
    Poco::Path libPath(Pothos::System::getRootPath());
    libPath.append("@CMAKE_INSTALL_LIBDIR@");
    return libPath.absolute().toString();
}

std::vector<std::string> Pothos::System::getPothosModuleSearchPaths()
{
    std::vector<Poco::Path> searchPaths;

    //the default search path
    Poco::Path devLibPath(Pothos::System::getPothosDevLibraryPath());
    devLibPath.append("Pothos");
    devLibPath.append("modules" + Pothos::System::getAbiVersion());
    searchPaths.push_back(devLibPath);

    //support /usr/local module installs when the install prefix is /usr
    if (Pothos::System::getRootPath() == "/usr")
    {
        searchPaths.push_back("/usr/local/@CMAKE_INSTALL_LIBDIR@/Pothos/modules" + Pothos::System::getAbiVersion());
        //when using a multi-arch directory, support single-arch path as well
        static const std::string libdir("@CMAKE_INSTALL_LIBDIR@");
        if (libdir.find("lib/") == 0) //startswith lib/ indicating multi-arch
            searchPaths.push_back("/usr/local/lib/Pothos/modules" + Pothos::System::getAbiVersion());

    }

    //separator for search paths
    const std::string sep(1, Poco::Path::pathSeparator());

    //check the environment's search path
    const auto pluginPaths = Poco::Environment::get("POTHOS_PLUGIN_PATH", "");
    for (const auto &pluginPath : Poco::StringTokenizer(pluginPaths, sep))
    {
        if (pluginPath.empty()) continue;
        searchPaths.push_back(Poco::Path(pluginPath));
    }

    std::vector<std::string> searchPathStrings;
    std::transform(
        searchPaths.begin(),
        searchPaths.end(),
        std::back_inserter(searchPathStrings),
        [](const Poco::Path& path)
        {
            return path.absolute().toString();
        });

    return searchPathStrings;
}
