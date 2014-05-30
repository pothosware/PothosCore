// Copyright (c) 2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Object/Containers.hpp>
#include <Pothos/Proxy/Containers.hpp>
#include <Pothos/Plugin.hpp>
#include <Pothos/Callable.hpp>

static Pothos::ObjectVector convertProxyVectorToObjectVector(const Pothos::ProxyVector &v)
{
    Pothos::ObjectVector objVec;
    for (const auto &elem : v)
    {
        auto o = elem.getEnvironment()->convertProxyToObject(elem);
        objVec.push_back(o);
    }
    return objVec;
}

static Pothos::ObjectSet convertProxySetToObjectSet(const Pothos::ProxySet &s)
{
    Pothos::ObjectSet objSet;
    for (const auto &elem : s)
    {
        auto o = elem.getEnvironment()->convertProxyToObject(elem);
        objSet.insert(o);
    }
    return objSet;
}

static Pothos::ObjectMap convertProxyMapToObjectMap(const Pothos::ProxyMap &m)
{
    Pothos::ObjectMap objMap;
    for (const auto &elem : m)
    {
        auto k = elem.first.getEnvironment()->convertProxyToObject(elem.first);
        auto v = elem.second.getEnvironment()->convertProxyToObject(elem.second);
        objMap[k] = v;
    }
    return objMap;
}

//TODO convert in the other direction

pothos_static_block(pothosObjectRegisterConvertContainers)
{
    Pothos::PluginRegistry::add("/object/convert/containers/proxy_vector_to_object_vector", Pothos::Callable(&convertProxyVectorToObjectVector));
    Pothos::PluginRegistry::add("/object/convert/containers/proxy_set_to_object_set", Pothos::Callable(&convertProxySetToObjectSet));
    Pothos::PluginRegistry::add("/object/convert/containers/proxy_map_to_object_map", Pothos::Callable(&convertProxyMapToObjectMap));
}
