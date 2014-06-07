// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PothosModule.hpp"
#include <Pothos/Plugin.hpp>

/***********************************************************************
 * Proxy conversions
 **********************************************************************/
static Pothos::Proxy convertPyProxyToProxy(const Pothos::Proxy &proxy)
{
    PyObjectRef ref(ProxyToPyObject(proxy), REF_NEW);
    return *reinterpret_cast<ProxyObject *>(ref.obj)->proxy;
}

/***********************************************************************
 * Object conversions
 **********************************************************************/
static Pothos::Proxy convertObjectToPyObject(Pothos::ProxyEnvironment::Sptr env, const Pothos::Object &obj)
{
    //try to convert the object to a native python type
    try
    {
        return env->convertObjectToProxy(obj);
    }
    //otherwise make a proxy of the given object
    catch (const Pothos::ProxyEnvironmentConvertError &)
    {
        auto managed = Pothos::ProxyEnvironment::make("managed");
        auto proxy = managed->convertObjectToProxy(obj);
        return env->makeProxy(proxy);
    }
}

/***********************************************************************
 * Register all conversions
 **********************************************************************/
void registerPothosModuleConverters(void)
{
    Pothos::PluginRegistry::addCall("/proxy/converters/python/proxy_to_pyproxy",
        &convertProxyToPyProxy);
    Pothos::PluginRegistry::add("/proxy/converters/python/pyproxy_to_proxy",
        Pothos::ProxyConvertPair("PothosProxy", &convertPyProxyToProxy));

    Pothos::PluginRegistry::addCall("/proxy/converters/python/object_to_pyobject",
        &convertObjectToPyObject);
}
