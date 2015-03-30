// Copyright (c) 2014-2015 Josh Blum
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
    for (const auto &elem : v)
    {
        nativeVec.push_back(elem.convert<OutType>());
    }
    return nativeVec;
}

template <typename InType>
Pothos::ProxyVector convertNativeVectorToProxyVector(const std::vector<InType> &v)
{
    auto env = Pothos::ProxyEnvironment::make("managed");
    Pothos::ProxyVector pVec;
    for (const auto &elem : v)
    {
        auto o = env->makeProxy(elem);
        pVec.push_back(o);
    }
    return pVec;
}

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

template <typename KeyType, typename ValType>
std::map<KeyType, ValType> convertProxyMapToNativeMap(const Pothos::ProxyMap &m)
{
    std::map<KeyType, ValType> out;
    for (const auto &pair : m)
    {
        out[pair.first.convert<KeyType>()] = pair.second.convert<ValType>();
    }
    return out;
}

static Pothos::ProxyVector convertObjectVectorToProxyVector(const Pothos::ObjectVector &v)
{
    auto env = Pothos::ProxyEnvironment::make("managed");
    Pothos::ProxyVector pVec;
    for (const auto &elem : v)
    {
        auto o = env->convertObjectToProxy(elem);
        pVec.push_back(o);
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
        pSet.insert(o);
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
        pMap[k] = v;
    }
    return pMap;
}

template <typename T>
void registerNumericProxyVectorConversion(const std::string &name)
{
    Pothos::PluginRegistry::add("/object/convert/containers/proxy_vec_to_"+name+"_vec", Pothos::Callable(&convertProxyVectorToNativeVector<T>));
    Pothos::PluginRegistry::add("/object/convert/containers/"+name+"_vec_to_proxy_vec", Pothos::Callable(&convertNativeVectorToProxyVector<T>));
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

    Pothos::PluginRegistry::add("/object/convert/containers/object_vec_to_proxy_vec", Pothos::Callable(&convertObjectVectorToProxyVector));
    Pothos::PluginRegistry::add("/object/convert/containers/object_set_to_proxy_set", Pothos::Callable(&convertObjectSetToProxySet));
    Pothos::PluginRegistry::add("/object/convert/containers/object_map_to_proxy_map", Pothos::Callable(&convertObjectMapToProxyMap));
}
