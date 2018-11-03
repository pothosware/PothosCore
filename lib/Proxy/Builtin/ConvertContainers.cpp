// Copyright (c) 2014-2018 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Object/Containers.hpp>
#include <Pothos/Proxy.hpp>
#include <Pothos/Plugin.hpp>
#include <Pothos/Callable.hpp>
#include <string>
#include <complex>

template <typename OutType>
std::vector<OutType> convertProxyVectorToNativeVector(const Pothos::ProxyVector &v)
{
    std::vector<OutType> nativeVec;
    nativeVec.reserve(v.size());
    for (const auto &elem : v)
    {
        nativeVec.push_back(elem);
    }
    return nativeVec;
}

template <typename InType>
Pothos::ProxyVector convertNativeVectorToProxyVector(const std::vector<InType> &v)
{
    auto env = Pothos::ProxyEnvironment::make("managed");
    Pothos::ProxyVector pVec;
    pVec.reserve(v.size());
    for (const auto &elem : v)
    {
        auto o = env->makeProxy(elem);
        pVec.push_back(std::move(o));
    }
    return pVec;
}

static Pothos::ObjectVector convertProxyVectorToObjectVector(const Pothos::ProxyVector &v)
{
    Pothos::ObjectVector objVec;
    objVec.reserve(v.size());
    for (const auto &elem : v)
    {
        objVec.push_back(elem.toObject());
    }
    return objVec;
}

static Pothos::ObjectSet convertProxySetToObjectSet(const Pothos::ProxySet &s)
{
    Pothos::ObjectSet objSet;
    for (const auto &elem : s)
    {
        objSet.emplace_hint(objSet.end(), elem.toObject());
    }
    return objSet;
}

static Pothos::ObjectMap convertProxyMapToObjectMap(const Pothos::ProxyMap &m)
{
    Pothos::ObjectMap objMap;
    for (const auto &elem : m)
    {
        objMap.emplace_hint(objMap.end(), elem.first.toObject(), elem.second.toObject());
    }
    return objMap;
}

template <typename KeyType, typename ValType>
std::map<KeyType, ValType> convertProxyMapToNativeMap(const Pothos::ProxyMap &m)
{
    std::map<KeyType, ValType> out;
    for (const auto &pair : m)
    {
        out.emplace_hint(out.end(), pair.first, pair.second.convert<ValType>());
    }
    return out;
}

static Pothos::ProxyVector convertObjectVectorToProxyVector(const Pothos::ObjectVector &v)
{
    auto env = Pothos::ProxyEnvironment::make("managed");
    Pothos::ProxyVector pVec;
    pVec.reserve(v.size());
    for (const auto &elem : v)
    {
        auto o = env->convertObjectToProxy(elem);
        pVec.push_back(std::move(o));
    }
    return pVec;
}

static Pothos::ProxySet convertObjectSetToProxySet(const Pothos::ObjectSet &s)
{
    auto env = Pothos::ProxyEnvironment::make("managed");
    Pothos::ProxySet pSet;
    for (const auto &elem : s)
    {
        auto o = env->convertObjectToProxy(elem);
        pSet.emplace_hint(pSet.end(), std::move(o));
    }
    return pSet;
}

static Pothos::ProxyMap convertObjectMapToProxyMap(const Pothos::ObjectMap &m)
{
    auto env = Pothos::ProxyEnvironment::make("managed");
    Pothos::ProxyMap pMap;
    for (const auto &elem : m)
    {
        auto k = env->convertObjectToProxy(elem.first);
        auto v = env->convertObjectToProxy(elem.second);
        pMap.emplace_hint(pMap.end(), std::move(k), std::move(v));
    }
    return pMap;
}

template <typename T>
void registerNumericProxyVectorConversion(const std::string &name)
{
    Pothos::PluginRegistry::add("/object/convert/containers/proxy_vec_to_"+name+"_vec", Pothos::Callable(&convertProxyVectorToNativeVector<T>));
    Pothos::PluginRegistry::add("/object/convert/containers/"+name+"_vec_to_proxy_vec", Pothos::Callable(&convertNativeVectorToProxyVector<T>));
    Pothos::PluginRegistry::add("/object/convert/containers/proxy_vec_to_"+name+"_matrix", Pothos::Callable(&convertProxyVectorToNativeVector<std::vector<T>>));
    Pothos::PluginRegistry::add("/object/convert/containers/"+name+"_matrix_to_proxy_vec", Pothos::Callable(&convertNativeVectorToProxyVector<std::vector<T>>));
}

static Pothos::ObjectKwargs convertObjectMapToObjectKwargs(const Pothos::ObjectMap &oMap)
{
    Pothos::ObjectKwargs kwargs;
    for (const auto &pair : oMap)
    {
        kwargs.emplace_hint(kwargs.end(), pair.first.toString(), pair.second);
    }
    return kwargs;
}

static Pothos::ObjectMap convertObjectKwargsToObjectMap(const Pothos::ObjectKwargs &kwargs)
{
    Pothos::ObjectMap oMap;
    for (const auto &pair : kwargs)
    {
        oMap.emplace_hint(oMap.end(), pair.first, pair.second);
    }
    return oMap;
}

pothos_static_block(pothosObjectRegisterConvertContainers)
{
    registerNumericProxyVectorConversion<bool>("bool");
    registerNumericProxyVectorConversion<char>("char");
    registerNumericProxyVectorConversion<signed char>("schar");
    registerNumericProxyVectorConversion<unsigned char>("uchar");
    registerNumericProxyVectorConversion<signed short>("sshort");
    registerNumericProxyVectorConversion<unsigned short>("ushort");
    registerNumericProxyVectorConversion<signed int>("sint");
    registerNumericProxyVectorConversion<unsigned int>("uint");
    registerNumericProxyVectorConversion<signed long>("slong");
    registerNumericProxyVectorConversion<unsigned long>("ulong");
    registerNumericProxyVectorConversion<signed long long>("sllong");
    registerNumericProxyVectorConversion<unsigned long long>("ullong");
    registerNumericProxyVectorConversion<float>("float");
    registerNumericProxyVectorConversion<double>("double");
    registerNumericProxyVectorConversion<std::complex<float>>("cfloat");
    registerNumericProxyVectorConversion<std::complex<double>>("cdouble");
    registerNumericProxyVectorConversion<std::string>("string");

    Pothos::PluginRegistry::add("/object/convert/containers/proxy_vec_to_object_vec", Pothos::Callable(&convertProxyVectorToObjectVector));
    Pothos::PluginRegistry::add("/object/convert/containers/proxy_set_to_object_set", Pothos::Callable(&convertProxySetToObjectSet));
    Pothos::PluginRegistry::add("/object/convert/containers/proxy_map_to_object_map", Pothos::Callable(&convertProxyMapToObjectMap));
    Pothos::PluginRegistry::add("/object/convert/containers/proxy_map_to_string_map", Pothos::Callable(&convertProxyMapToNativeMap<std::string, std::string>));
    Pothos::PluginRegistry::add("/object/convert/containers/proxy_map_to_kwargs", Pothos::Callable(&convertProxyMapToNativeMap<std::string, Pothos::Object>));

    Pothos::PluginRegistry::add("/object/convert/containers/object_vec_to_proxy_vec", Pothos::Callable(&convertObjectVectorToProxyVector));
    Pothos::PluginRegistry::add("/object/convert/containers/object_set_to_proxy_set", Pothos::Callable(&convertObjectSetToProxySet));
    Pothos::PluginRegistry::add("/object/convert/containers/object_map_to_proxy_map", Pothos::Callable(&convertObjectMapToProxyMap));
    Pothos::PluginRegistry::add("/object/convert/containers/object_map_to_object_kwargs", Pothos::Callable(&convertObjectMapToObjectKwargs));
    Pothos::PluginRegistry::add("/object/convert/containers/object_kwargs_to_object_map", Pothos::Callable(&convertObjectKwargsToObjectMap));
}
