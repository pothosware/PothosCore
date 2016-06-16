// Copyright (c) 2013-2016 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Util/TypeInfo.hpp>

//demangle support for pretty strings
#ifdef __GNUG__
#include <cxxabi.h>
#define HAVE_CXA_DEMANGLE
#endif

std::string Pothos::Util::typeInfoToString(const std::type_info &type)
{
    //Since std::string is used a lot and often has a complicated template name,
    //we just enforce returning a simple display name for the std::string type.
    if (type == typeid(std::string)) return "std::string";

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
