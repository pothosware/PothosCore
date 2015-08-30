// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Plugin.hpp>
#include <Pothos/Proxy.hpp>

/***********************************************************************
 * Vector
 **********************************************************************/
static Pothos::Proxy convertVectorToJVector(Pothos::ProxyEnvironment::Sptr env, const Pothos::ProxyVector &vec)
{
    auto jVector = env->findProxy("java.util.Vector").callProxy("new");
    jVector.callVoid("setSize", vec.size());
    for (size_t i = 0; i < vec.size(); i++)
    {
        jVector.callVoid("set", i, vec[i]);
    }
    return jVector;
}

static Pothos::ProxyVector convertJVectorToVector(const Pothos::Proxy &proxy)
{
    Pothos::ProxyVector vec(proxy.call<size_t>("size"));
    for (size_t i = 0; i < vec.size(); i++)
    {
        vec[i] = proxy.callProxy("get", i);
    }
    return vec;
}

pothos_static_block(pothosRegisterJavaVectorConversions)
{
    Pothos::PluginRegistry::addCall("/proxy/converters/java/vector_to_jvector",
        &convertVectorToJVector);
    Pothos::PluginRegistry::add("/proxy/converters/java/jvector_to_vector",
        Pothos::ProxyConvertPair("java.util.Vector", &convertJVectorToVector));
}

/***********************************************************************
 * Set
 **********************************************************************/
static Pothos::Proxy convertSetToJSet(Pothos::ProxyEnvironment::Sptr env, const Pothos::ProxySet &set)
{
    auto jSet = env->findProxy("java.util.HashSet").callProxy("new");
    for (const auto &entry : set)
    {
        jSet.callVoid("add", entry);
    }
    return jSet;
}

static Pothos::ProxySet convertJSetToSet(const Pothos::Proxy &proxy)
{
    Pothos::ProxySet set;
    auto it = proxy.callProxy("iterator");
    while (it.call<bool>("hasNext"))
    {
        set.insert(it.callProxy("next"));
    }
    return set;
}

pothos_static_block(pothosRegisterJavaSetConversions)
{
    Pothos::PluginRegistry::addCall("/proxy/converters/java/set_to_jset",
        &convertSetToJSet);
    Pothos::PluginRegistry::add("/proxy/converters/java/jset_to_set",
        Pothos::ProxyConvertPair("java.util.HashSet", &convertJSetToSet));
}

/***********************************************************************
 * Map
 **********************************************************************/
static Pothos::Proxy convertMapToJMap(Pothos::ProxyEnvironment::Sptr env, const Pothos::ProxyMap &map)
{
    auto jMap = env->findProxy("java.util.HashMap").callProxy("new");
    for (const auto &entry : map)
    {
        jMap.callVoid("put", entry.first, entry.second);
    }
    return jMap;
}

static Pothos::ProxyMap convertJMapToMap(const Pothos::Proxy &proxy)
{
    Pothos::ProxyMap map;
    auto it = proxy.callProxy("entrySet").callProxy("iterator");
    while (it.call<bool>("hasNext"))
    {
        auto entry = it.callProxy("next");
        map[entry.callProxy("getKey")] = entry.callProxy("getValue");
    }
    return map;
}

pothos_static_block(pothosRegisterJavaMapConversions)
{
    Pothos::PluginRegistry::addCall("/proxy/converters/java/map_to_jmap",
        &convertMapToJMap);
    Pothos::PluginRegistry::add("/proxy/converters/java/jmap_to_map",
        Pothos::ProxyConvertPair("java.util.HashMap", &convertJMapToMap));
}
