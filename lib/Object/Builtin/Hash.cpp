// Copyright (c) 2013-2013 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Plugin.hpp>
#include <functional> //std::hash
#include <string>

static size_t hashNullObject(const Pothos::NullObject &obj)
{
    return typeid(obj).hash_code(); //its always the hash of the type, not the contents
}

template <typename T>
static size_t hashFunction(const T &v)
{
    return std::hash<T>()(v);
}

pothos_static_block(pothosObjectRegisterHash)
{
    Pothos::PluginRegistry::addCall("/object/hash/null", &hashNullObject);
    Pothos::PluginRegistry::addCall("/object/hash/bool", &hashFunction<bool>);
    Pothos::PluginRegistry::addCall("/object/hash/char", &hashFunction<char>);
    Pothos::PluginRegistry::addCall("/object/hash/schar", &hashFunction<signed char>);
    Pothos::PluginRegistry::addCall("/object/hash/uchar", &hashFunction<unsigned char>);
    Pothos::PluginRegistry::addCall("/object/hash/sshort", &hashFunction<signed short>);
    Pothos::PluginRegistry::addCall("/object/hash/ushort", &hashFunction<unsigned short>);
    Pothos::PluginRegistry::addCall("/object/hash/sint", &hashFunction<signed int>);
    Pothos::PluginRegistry::addCall("/object/hash/uint", &hashFunction<unsigned int>);
    Pothos::PluginRegistry::addCall("/object/hash/slong", &hashFunction<signed long>);
    Pothos::PluginRegistry::addCall("/object/hash/ulong", &hashFunction<unsigned long>);
    Pothos::PluginRegistry::addCall("/object/hash/sllong", &hashFunction<signed long long>);
    Pothos::PluginRegistry::addCall("/object/hash/ullong", &hashFunction<unsigned long long>);
    Pothos::PluginRegistry::addCall("/object/hash/float", &hashFunction<float>);
    Pothos::PluginRegistry::addCall("/object/hash/double", &hashFunction<double>);
    Pothos::PluginRegistry::addCall("/object/hash/string", &hashFunction<std::string>);
}
