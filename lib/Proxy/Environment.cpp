// Copyright (c) 2013-2017 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Proxy/Exception.hpp>
#include <Pothos/Proxy/Environment.hpp>
#include <Pothos/Callable.hpp>
#include <Pothos/Plugin.hpp>
#include <Pothos/System/HostInfo.hpp>

Pothos::ProxyEnvironment::Sptr Pothos::ProxyEnvironment::make(const std::string &name, const ProxyEnvironmentArgs &args)
{
    Sptr environment;
    try
    {
        const auto plugin = Pothos::PluginRegistry::get(Pothos::PluginPath("/proxy/environment").join(name));
        const auto &callable = plugin.getObject().extract<Pothos::Callable>();
        environment = callable.call(args);
    }
    catch(const Exception &ex)
    {
        throw Pothos::ProxyEnvironmentFactoryError("Pothos::ProxyEnvironment::make("+name+")", ex);
    }
    return environment;
}

Pothos::ProxyEnvironment::~ProxyEnvironment(void)
{
    return;
}

std::string Pothos::ProxyEnvironment::getNodeId(void) const
{
    return System::HostInfo::get().nodeId;
}

std::string Pothos::ProxyEnvironment::getUniquePid(void) const
{
    return getLocalUniquePid();
}

std::string Pothos::ProxyEnvironment::getLocalUniquePid(void)
{
    const auto info = Pothos::System::HostInfo::get();
    return info.nodeName + "/" + info.nodeId + "/" + info.pid;
}

std::string Pothos::ProxyEnvironment::getPeeringAddress(void)
{
    //not a real address for the local environment
    return "local";
}

#include <Pothos/Managed.hpp>

static auto managedProxyEnvironment = Pothos::ManagedClass()
    .registerClass<Pothos::ProxyEnvironment>()
    .registerStaticMethod(POTHOS_FCN_TUPLE(Pothos::ProxyEnvironment, make))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::ProxyEnvironment, getName))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::ProxyEnvironment, findProxy))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::ProxyEnvironment, getNodeId))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::ProxyEnvironment, getUniquePid))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::ProxyEnvironment, getLocalUniquePid))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::ProxyEnvironment, getPeeringAddress))
    .commit("Pothos/ProxyEnvironment");
