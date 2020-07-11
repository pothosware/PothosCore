// Copyright (c) 2020 Nicholas Corgan
// SPDX-License-Identifier: BSL-1.0

#include "PothosUtil.hpp"

#include <Pothos/Callable.hpp>
#include <Pothos/Plugin.hpp>
#include <Pothos/Proxy.hpp>
#include <Pothos/System/Paths.hpp>
#include <Pothos/Util/TypeInfo.hpp>

#include <Poco/Format.h>
#include <Poco/Path.h>

#include <iostream>
#include <vector>

// Expected signature: Pothos::Proxy(Pothos::ProxyEnvironment::Sptr, const CppType&)
static bool isCallableConverterToEnv(const Pothos::Callable& callable)
{
    if (callable.getNumArgs() != 2) return false;
    if (callable.type(-1) != typeid(Pothos::Proxy)) return false;
    if (callable.type(0) != typeid(Pothos::ProxyEnvironment::Sptr)) return false;

    return true;
}

// Expected signature: CppType(const Pothos::Proxy&)
static bool isCallableConverterFromEnv(const Pothos::Callable& callable)
{
    if (callable.getNumArgs() != 1) return false;
    if (callable.type(0) != typeid(const Pothos::Proxy&)) return false;

    return true;
}

static std::string getEnvironmentModuleFilepath(const std::string& proxyEnvName)
{
    if ((proxyEnvName == "managed") || (proxyEnvName == "remote")) return Pothos::System::getPothosRuntimeLibraryPath();

    // Since we can't necessarily assume there are are specific converters or tests,
    // look in all potential locations for a plugin registered in paths designated
    // for proxy-specific plugins.
    const auto convertersPluginPath = "/proxy/converters/" + proxyEnvName;
    const auto testsPluginPath = Poco::format("/proxy/%s/tests", proxyEnvName);

    for (const auto& pluginLeaf : Pothos::PluginRegistry::list(convertersPluginPath))
    {
        const auto pluginPath = Poco::format("%s/%s", convertersPluginPath, pluginLeaf);
        const auto plugin = Pothos::PluginRegistry::get(pluginPath);

        return plugin.getModule().getFilePath();
    }
    for (const auto& pluginLeaf : Pothos::PluginRegistry::list(testsPluginPath))
    {
        const auto pluginPath = Poco::format("%s/%s", testsPluginPath, pluginLeaf);
        const auto plugin = Pothos::PluginRegistry::get(pluginPath);

        return plugin.getModule().getFilePath();
    }

    return "";
}

static void printConverters(const std::string& proxyEnvName)
{
    const auto convertersPluginPath = "/proxy/converters/" + proxyEnvName;

    if (Pothos::PluginRegistry::exists(convertersPluginPath))
    {
        std::vector<Pothos::Callable> convertersToEnvironment;
        std::vector<Pothos::ProxyConvertPair> convertersFromEnvironment;

        for (const auto& pluginLeaf : Pothos::PluginRegistry::list(convertersPluginPath))
        {
            const auto pluginPath = Poco::format("%s/%s", convertersPluginPath, pluginLeaf);
            const auto plugin = Pothos::PluginRegistry::get(pluginPath);

            if (plugin.getObject().type() == typeid(Pothos::Callable))
            {
                auto pluginCallable = plugin.getObject().extract<Pothos::Callable>();
                if (isCallableConverterToEnv(pluginCallable)) convertersToEnvironment.emplace_back(std::move(pluginCallable));
            }
            else if (plugin.getObject().type() == typeid(Pothos::ProxyConvertPair))
            {
                auto pluginPair = plugin.getObject().extract<Pothos::ProxyConvertPair>();
                if (isCallableConverterFromEnv(pluginPair.second)) convertersFromEnvironment.emplace_back(std::move(pluginPair));
            }
        }

        if (!convertersToEnvironment.empty())
        {
            std::cout << std::endl << "Types directly convertible to environment:" << std::endl;
            for (const auto& converterCallable : convertersToEnvironment)
            {
                std::cout << " * " << Pothos::Util::typeInfoToString(converterCallable.type(1)) << std::endl;
            }
        }
        if (!convertersFromEnvironment.empty())
        {
            std::cout << std::endl << "Type conversions from environment:" << std::endl;
            for (const auto& converterPair : convertersFromEnvironment)
            {
                std::cout << " * " << converterPair.first << " -> " << Pothos::Util::typeInfoToString(converterPair.second.type(-1)) << std::endl;
            }
        }
    }
}

static void printTests(const std::string& proxyEnvName)
{
    const auto testsPluginPath = Poco::format("/proxy/%s/tests", proxyEnvName);

    if (Pothos::PluginRegistry::exists(testsPluginPath))
    {
        std::cout << std::endl << "Self-tests:" << std::endl;
        for (const auto& pluginLeaf : Pothos::PluginRegistry::list(testsPluginPath))
        {
            std::cout << " * " << pluginLeaf << std::endl;
        }
    }
}

void PothosUtilBase::printProxyEnvironmentInfo(const std::string&, const std::string&)
{
    Pothos::ScopedInit init;

    const auto proxyEnvName = this->config().getString("proxyEnvName");
    if (proxyEnvName.empty())
    {
        std::cout << ">>> Specify --proxy-environment-name=proxyEnvName for more information..." << std::endl;
        std::cout << ">>> Available proxy environments are:" << std::endl;
        for (const auto& name : Pothos::PluginRegistry::list("/proxy"))
        {
            if (name != "converters") std::cout << "  * " << name << std::endl;
        }
        std::cout << std::endl;
        return;
    }

    std::cout << "Proxy environment " << proxyEnvName << " (" << getEnvironmentModuleFilepath(proxyEnvName) << ")" << std::endl;
    printConverters(proxyEnvName);
    printTests(proxyEnvName);
}
