// Copyright (c) 2014 Josh Blum
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

pothos_static_block(pothosObjectRegisterConvertContainers)
{
    Pothos::PluginRegistry::add("/object/convert/containers/proxy_vec_to_bool_vec", Pothos::Callable(&convertProxyVectorToNativeVector<bool>));
    Pothos::PluginRegistry::add("/object/convert/containers/proxy_vec_to_char_vec", Pothos::Callable(&convertProxyVectorToNativeVector<char>));
    Pothos::PluginRegistry::add("/object/convert/containers/proxy_vec_to_schar_vec", Pothos::Callable(&convertProxyVectorToNativeVector<signed char>));
    Pothos::PluginRegistry::add("/object/convert/containers/proxy_vec_to_uchar_vec", Pothos::Callable(&convertProxyVectorToNativeVector<unsigned char>));
    Pothos::PluginRegistry::add("/object/convert/containers/proxy_vec_to_sshort_vec", Pothos::Callable(&convertProxyVectorToNativeVector<signed short>));
    Pothos::PluginRegistry::add("/object/convert/containers/proxy_vec_to_ushort_vec", Pothos::Callable(&convertProxyVectorToNativeVector<unsigned short>));
    Pothos::PluginRegistry::add("/object/convert/containers/proxy_vec_to_sint_vec", Pothos::Callable(&convertProxyVectorToNativeVector<signed int>));
    Pothos::PluginRegistry::add("/object/convert/containers/proxy_vec_to_uint_vec", Pothos::Callable(&convertProxyVectorToNativeVector<unsigned int>));
    Pothos::PluginRegistry::add("/object/convert/containers/proxy_vec_to_slong_vec", Pothos::Callable(&convertProxyVectorToNativeVector<signed long>));
    Pothos::PluginRegistry::add("/object/convert/containers/proxy_vec_to_ulong_vec", Pothos::Callable(&convertProxyVectorToNativeVector<unsigned long>));
    Pothos::PluginRegistry::add("/object/convert/containers/proxy_vec_to_sllong_vec", Pothos::Callable(&convertProxyVectorToNativeVector<signed long long>));
    Pothos::PluginRegistry::add("/object/convert/containers/proxy_vec_to_ullong_vec", Pothos::Callable(&convertProxyVectorToNativeVector<unsigned long long>));
    Pothos::PluginRegistry::add("/object/convert/containers/proxy_vec_to_float_vec", Pothos::Callable(&convertProxyVectorToNativeVector<float>));
    Pothos::PluginRegistry::add("/object/convert/containers/proxy_vec_to_double_vec", Pothos::Callable(&convertProxyVectorToNativeVector<double>));
    Pothos::PluginRegistry::add("/object/convert/containers/proxy_vec_to_cfloat_vec", Pothos::Callable(&convertProxyVectorToNativeVector<std::complex<float>>));
    Pothos::PluginRegistry::add("/object/convert/containers/proxy_vec_to_cdouble_vec", Pothos::Callable(&convertProxyVectorToNativeVector<std::complex<double>>));
    Pothos::PluginRegistry::add("/object/convert/containers/proxy_vec_to_string_vec", Pothos::Callable(&convertProxyVectorToNativeVector<std::string>));

    Pothos::PluginRegistry::add("/object/convert/containers/proxy_vec_to_object_vec", Pothos::Callable(&convertProxyVectorToObjectVector));
    Pothos::PluginRegistry::add("/object/convert/containers/proxy_set_to_object_set", Pothos::Callable(&convertProxySetToObjectSet));
    Pothos::PluginRegistry::add("/object/convert/containers/proxy_map_to_object_map", Pothos::Callable(&convertProxyMapToObjectMap));
    Pothos::PluginRegistry::add("/object/convert/containers/proxy_map_to_string_map", Pothos::Callable(&convertProxyMapToNativeMap<std::string, std::string>));

    Pothos::PluginRegistry::add("/object/convert/containers/object_vec_to_proxy_vec", Pothos::Callable(&convertObjectVectorToProxyVector));
    Pothos::PluginRegistry::add("/object/convert/containers/object_set_to_proxy_set", Pothos::Callable(&convertObjectSetToProxySet));
    Pothos::PluginRegistry::add("/object/convert/containers/object_map_to_proxy_map", Pothos::Callable(&convertObjectMapToProxyMap));
}
