// Copyright (c) 2013-2016 Josh Blum
//                    2021 Nicholas Corgan
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Object.hpp>
#include <Pothos/Object/Containers.hpp>
#include <Pothos/Proxy.hpp>
#include <Pothos/Util/TypeInfo.hpp>

#include <complex>
#include <cstdint>
#include <map>
#include <set>
#include <typeinfo>
#include <unordered_map>
#include <vector>

//demangle support for pretty strings
#ifdef __GNUG__
#include <cxxabi.h>
#define HAVE_CXA_DEMANGLE
#endif

std::string Pothos::Util::typeInfoToString(const std::type_info &type)
{
    #define TypeNameEntry(T) \
        {typeid(T).hash_code(), #T}

    #define ContainerTypeNameEntries(T) \
        TypeNameEntry(std::set<T>), \
        TypeNameEntry(std::vector<T>), \
        TypeNameEntry(std::vector<std::vector<T>>)

    #define NumericTypeNameEntries(T) \
        TypeNameEntry(T), \
        ContainerTypeNameEntries(T), \
        ContainerTypeNameEntries(std::complex<T>)

    static const std::unordered_map<size_t, std::string> CustomTypeNames =
    {
        TypeNameEntry(std::string),
        TypeNameEntry(Pothos::ObjectKwargs),
        TypeNameEntry(Pothos::ObjectMap),
        TypeNameEntry(Pothos::ObjectSet),
        TypeNameEntry(Pothos::ObjectVector),
        TypeNameEntry(Pothos::ProxyMap),
        TypeNameEntry(Pothos::ProxySet),
        TypeNameEntry(Pothos::ProxyVector),
        TypeNameEntry(Pothos::ProxyEnvironment::Sptr),
        TypeNameEntry(Pothos::ProxyEnvironmentArgs),
        TypeNameEntry(Pothos::ProxyConvertPair),
        NumericTypeNameEntries(bool),
        NumericTypeNameEntries(char),
        NumericTypeNameEntries(signed char),
        NumericTypeNameEntries(short),
        NumericTypeNameEntries(int),
        NumericTypeNameEntries(long),
        NumericTypeNameEntries(long long),
        NumericTypeNameEntries(unsigned char),
        NumericTypeNameEntries(unsigned short),
        NumericTypeNameEntries(unsigned int),
        NumericTypeNameEntries(unsigned long),
        NumericTypeNameEntries(unsigned long long),
        NumericTypeNameEntries(float),
        NumericTypeNameEntries(double),
    };

    auto typeNameIter = CustomTypeNames.find(type.hash_code());
    if(typeNameIter != CustomTypeNames.end()) return typeNameIter->second;

    const char *name = type.name();
    #ifdef HAVE_CXA_DEMANGLE
    char buff[1024];
    size_t size = 1024;
    int status = -1;
    char* res = abi::__cxa_demangle(name, buff, &size, &status);
    if (status == 0) return std::string(res);
    #endif
    return name;
}
