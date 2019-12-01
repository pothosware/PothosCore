// Copyright (c) 2014-2014 Josh Blum
//                    2019 Nicholas Corgan
// SPDX-License-Identifier: BSL-1.0

#include "ConvertCommonImpl.hpp"
#include <Pothos/Plugin.hpp>
#include <Pothos/Callable.hpp>
#include <Pothos/Exception.hpp>
#include <Poco/NumberParser.h>
#include <type_traits>
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
static std::string convertNumToString(const Type &value)
{
    return std::to_string(value);
}

template <typename Type>
typename std::enable_if<std::is_signed<Type>::value, Type>::type convertStringToLongNum(const std::string &s)
{
    static_assert(
        sizeof(Type) == sizeof(Poco::Int64),
        "sizeof(Type) != sizeof(Poco::Int64)");

    Poco::Int64 ret = 0;
    if(!Poco::NumberParser::tryParse64(s, ret))
    {
        throw Pothos::InvalidArgumentException(
                  "Invalid number string",
                  s);
    }

    return static_cast<Type>(ret);
}

template <typename Type>
typename std::enable_if<!std::is_signed<Type>::value, Type>::type convertStringToLongNum(const std::string &s)
{
    static_assert(
        sizeof(Type) == sizeof(Poco::UInt64),
        "sizeof(Type) != sizeof(Poco::UInt64)");

    Poco::UInt64 ret = 0;
    if(!Poco::NumberParser::tryParseUnsigned64(s, ret))
    {
        throw Pothos::InvalidArgumentException(
                  "Invalid unsigned number string",
                  s);
    }

    return static_cast<Type>(ret);
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
#define declare_string_long_conversion2(type) \
    registerConvertString("string", #type, &convertStringToLongNum<type>); \
    registerConvertString(#type, "string", &convertNumToString<type>);

/***********************************************************************
 * all registration code is called from here
 **********************************************************************/
pothos_static_block(pothosObjectRegisterConvertStrings)
{
    registerConvertString("byte_vector", "string", &convertByteVectorToString<char>);
    registerConvertString("signed_byte_vector", "string", &convertByteVectorToString<signed char>);
    registerConvertString("unsigned_byte_vector", "string", &convertByteVectorToString<unsigned char>);
    registerConvertString("string", "byte_vector", &convertStringToByteVector);
    declare_string_long_conversion2(slong);
    declare_string_long_conversion2(sllong);
    declare_string_long_conversion2(ulong);
    declare_string_long_conversion2(ullong);
}
