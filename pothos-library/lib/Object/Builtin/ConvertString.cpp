// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Plugin.hpp>
#include <Pothos/Callable.hpp>
#include <sstream>
#include <vector>

/***********************************************************************
 * byte vector to from string
 **********************************************************************/
template <typename ByteType>
std::string convertByteVectorToString(const std::vector<ByteType> &vec)
{
    return std::string(reinterpret_cast<const char *>(vec.data()), vec.size());
}

static std::vector<char> convertStringToByteVector(const std::string &s)
{
    return std::vector<char>(s.begin(), s.end());
}

/***********************************************************************
 * templated conversions to/from string
 **********************************************************************/
template <typename Type>
std::string convertToString(const Type &value)
{
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

template <typename Type>
Type convertFromString(const std::string &s)
{
    Type value = 0;
    std::istringstream iss(s);
    iss >> value;
    return value;
}

/***********************************************************************
 * helper function registers a converter for specific types
 **********************************************************************/
static void registerConvertString(const std::string &inName, const std::string &outName, const Pothos::Callable &callable)
{
    const std::string name = inName + "_to_" + outName;
    auto path = Pothos::PluginPath("/object/convert/strings").join(name);
    Pothos::PluginRegistry::add(path, callable);
}

/***********************************************************************
 * macros to declare all conversions
 **********************************************************************/
#define declare_string_conversion2(type) \
    registerConvertString("string", #type, &convertFromString<type>); \
    registerConvertString(#type, "string", &convertToString<type>);

/***********************************************************************
 * all registration code is called from here
 **********************************************************************/
pothos_static_block(pothosObjectRegisterConvertStrings)
{
    registerConvertString("byte_vector", "string", &convertByteVectorToString<char>);
    registerConvertString("signed_byte_vector", "string", &convertByteVectorToString<signed char>);
    registerConvertString("unsigned_byte_vector", "string", &convertByteVectorToString<unsigned char>);
    registerConvertString("string", "byte_vector", &convertStringToByteVector);
    //no integer to string conversions that are non-implicit in a regular language
    /*
    declare_string_conversion2(char);
    declare_string_conversion2(schar);
    declare_string_conversion2(uchar);
    declare_string_conversion2(sshort);
    declare_string_conversion2(ushort);
    declare_string_conversion2(sint);
    declare_string_conversion2(uint);
    declare_string_conversion2(slong);
    declare_string_conversion2(ulong);
    declare_string_conversion2(sllong);
    declare_string_conversion2(ullong);
    declare_string_conversion2(float);
    declare_string_conversion2(double);
    */
}
