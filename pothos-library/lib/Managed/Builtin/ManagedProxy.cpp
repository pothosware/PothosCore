// Copyright (c) 2013-2013 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "ManagedProxy.hpp"
#include <Pothos/Plugin.hpp>
#include <Pothos/Managed.hpp>

ManagedProxyEnvironment::ManagedProxyEnvironment(const Pothos::ProxyEnvironmentArgs &)
{
    return;
}

Pothos::Proxy ManagedProxyEnvironment::makeHandle(const Pothos::Object &obj)
{
    auto env = std::dynamic_pointer_cast<ManagedProxyEnvironment>(this->shared_from_this());
    return Pothos::Proxy(new ManagedProxyHandle(env, obj));
}

std::shared_ptr<ManagedProxyHandle> ManagedProxyEnvironment::getHandle(const Pothos::Proxy &proxy)
{
    Pothos::Proxy myProxy = proxy;
    if (proxy.getEnvironment() != this->shared_from_this())
    {
        auto local = proxy.getEnvironment()->convertProxyToObject(proxy);
        myProxy = this->convertObjectToProxy(local);
    }
    return std::dynamic_pointer_cast<ManagedProxyHandle>(myProxy.getHandle());
}

Pothos::Proxy ManagedProxyEnvironment::findProxy(const std::string &name)
{
    Pothos::Plugin plugin;
    try
    {
        plugin = Pothos::PluginRegistry::get(Pothos::PluginPath("/managed").join(name));
    }
    catch(const Pothos::Exception &ex)
    {
        throw Pothos::ProxyEnvironmentFindError("ManagedProxyEnvironment::findProxy("+name+")", ex);
    }

    if (plugin.getObject().type() != typeid(Pothos::ManagedClass))
    {
        throw Pothos::ProxyEnvironmentFindError("ManagedProxyEnvironment::findProxy("+name+")", "plugin is not a managed class");
    }

    return this->makeHandle(plugin.getObject());
}

Pothos::Proxy ManagedProxyEnvironment::convertObjectToProxy(const Pothos::Object &local)
{
    return this->makeHandle(local);
}

Pothos::Object ManagedProxyEnvironment::convertProxyToObject(const Pothos::Proxy &proxy)
{
    auto handle = this->getHandle(proxy);
    return handle->obj;
}

/***********************************************************************
 * factory registration
 **********************************************************************/
Pothos::ProxyEnvironment::Sptr makeManagedProxyEnvironment(const Pothos::ProxyEnvironmentArgs &args)
{
    return Pothos::ProxyEnvironment::Sptr(new ManagedProxyEnvironment(args));
}

pothos_static_block(pothosRegisterManagedProxy)
{
    Pothos::PluginRegistry::addCall(
        "/proxy/environment/managed",
        &makeManagedProxyEnvironment);
}
