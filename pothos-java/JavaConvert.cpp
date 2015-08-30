// Copyright (c) 2013-2013 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Plugin.hpp>
#include "JavaProxy.hpp"
#include <Poco/Bugcheck.h>
#include <cstdint>
#include <cassert>

/***********************************************************************
 * Null
 **********************************************************************/
static Pothos::Proxy convertNullToJNull(Pothos::ProxyEnvironment::Sptr env, const Pothos::NullObject &)
{
    return std::dynamic_pointer_cast<JavaProxyEnvironment>(env)->makeHandle((jobject)nullptr);
}

static Pothos::NullObject convertJNullToNull(const Pothos::Proxy &)
{
    return Pothos::NullObject();
}

pothos_static_block(pothosRegisterJavaNullConversions)
{
    Pothos::PluginRegistry::addCall("/proxy/converters/java/null_to_jnull",
        &convertNullToJNull);
    Pothos::PluginRegistry::add("/proxy/converters/java/jnull_to_null",
        Pothos::ProxyConvertPair("", &convertJNullToNull));
}

/***********************************************************************
 * jvalue templated helpers
 **********************************************************************/
template <char sig, typename T>
static jvalue numToJvalue(const T &num)
{
    jvalue value;
    switch (sig) //a good compiler should simplify out the switch
    {
    case 'Z': value.z = jboolean(num); return value;
    case 'C': value.c = jchar(num); return value;
    case 'B': value.b = jbyte(num); return value;
    case 'S': value.s = jshort(num); return value;
    case 'I': value.i = jint(num); return value;
    case 'J': value.j = jlong(num); return value;
    case 'F': value.f = jfloat(num); return value;
    case 'D': value.d = jdouble(num); return value;
    }
    poco_bugcheck_msg(std::string(1, sig).c_str()); throw;
}

template <char sig, typename T>
static T jvalueToNum(const jvalue &value)
{
    switch (sig) //a good compiler should simplify out the switch
    {
    case 'Z': return T(value.z);
    case 'C': return T(value.c);
    case 'B': return T(value.b);
    case 'S': return T(value.s);
    case 'I': return T(value.i);
    case 'J': return T(value.j);
    case 'F': return T(value.f);
    case 'D': return T(value.d);
    }
    poco_bugcheck_msg(std::string(1, sig).c_str()); throw;
}

/***********************************************************************
 * Convert to Java Primitives
 **********************************************************************/
template <char sig, typename T>
static Pothos::Proxy convertNumToJPrimitive(Pothos::ProxyEnvironment::Sptr env, const T &num)
{
    return std::dynamic_pointer_cast<JavaProxyEnvironment>(env)->makeHandle(numToJvalue<sig>(num), sig);
}

template <char sig, typename T>
static T convertJPrimitiveToNum(const Pothos::Proxy &proxy)
{
    auto handle = std::dynamic_pointer_cast<JavaProxyHandle>(proxy.getHandle());
    assert(sig == handle->sig);
    return jvalueToNum<sig, T>(handle->value);
}

pothos_static_block(pothosRegisterJavaPrimitiveConversions)
{
    Pothos::PluginRegistry::addCall("/proxy/converters/java/bool_to_jboolean",
        &convertNumToJPrimitive<'Z', bool>);
    Pothos::PluginRegistry::add("/proxy/converters/java/jboolean_to_bool",
        Pothos::ProxyConvertPair("Z", &convertJPrimitiveToNum<'Z', bool>));

    Pothos::PluginRegistry::addCall("/proxy/converters/java/char_to_jchar",
        &convertNumToJPrimitive<'C', char>);
    Pothos::PluginRegistry::add("/proxy/converters/java/jchar_to_char",
        Pothos::ProxyConvertPair("C", &convertJPrimitiveToNum<'C', char>));

    Pothos::PluginRegistry::addCall("/proxy/converters/java/schar_to_jbyte",
        &convertNumToJPrimitive<'B', signed char>);
    Pothos::PluginRegistry::addCall("/proxy/converters/java/uchar_to_jbyte",
        &convertNumToJPrimitive<'B', unsigned char>);
    Pothos::PluginRegistry::add("/proxy/converters/java/jbyte_to_int8",
        Pothos::ProxyConvertPair("B", &convertJPrimitiveToNum<'B', int8_t>));

    Pothos::PluginRegistry::addCall("/proxy/converters/java/sshort_to_jshort",
        &convertNumToJPrimitive<'S', signed short>);
    Pothos::PluginRegistry::addCall("/proxy/converters/java/ushort_to_jshort",
        &convertNumToJPrimitive<'S', unsigned short>);
    Pothos::PluginRegistry::add("/proxy/converters/java/jshort_to_int16",
        Pothos::ProxyConvertPair("S", &convertJPrimitiveToNum<'S', int16_t>));

    Pothos::PluginRegistry::addCall("/proxy/converters/java/sint_to_jint",
        &convertNumToJPrimitive<'I', signed int>);
    Pothos::PluginRegistry::addCall("/proxy/converters/java/uint_to_jint",
        &convertNumToJPrimitive<'I', unsigned int>);
    Pothos::PluginRegistry::add("/proxy/converters/java/jint_to_int32",
        Pothos::ProxyConvertPair("I", &convertJPrimitiveToNum<'I', int32_t>));

    #ifndef POCO_LONG_IS_64_BIT
    Pothos::PluginRegistry::addCall("/proxy/converters/java/slong_to_jint",
        &convertNumToJPrimitive<'I', signed long>);
    Pothos::PluginRegistry::addCall("/proxy/converters/java/ulong_to_jint",
        &convertNumToJPrimitive<'I', unsigned long>);
    #else
    Pothos::PluginRegistry::addCall("/proxy/converters/java/slong_to_jlong",
        &convertNumToJPrimitive<'J', signed long>);
    Pothos::PluginRegistry::addCall("/proxy/converters/java/ulong_to_jlong",
        &convertNumToJPrimitive<'J', unsigned long>);
    #endif

    Pothos::PluginRegistry::addCall("/proxy/converters/java/sllong_to_jlong",
        &convertNumToJPrimitive<'J', signed long long>);
    Pothos::PluginRegistry::addCall("/proxy/converters/java/ullong_to_jlong",
        &convertNumToJPrimitive<'J', unsigned long long>);
    Pothos::PluginRegistry::add("/proxy/converters/java/jlong_to_int64",
        Pothos::ProxyConvertPair("J", &convertJPrimitiveToNum<'J', int64_t>));

    Pothos::PluginRegistry::addCall("/proxy/converters/java/float_to_jfloat",
        &convertNumToJPrimitive<'F', float>);
    Pothos::PluginRegistry::add("/proxy/converters/java/jfloat_to_float",
        Pothos::ProxyConvertPair("F", &convertJPrimitiveToNum<'F', float>));

    Pothos::PluginRegistry::addCall("/proxy/converters/java/double_to_jdouble",
        &convertNumToJPrimitive<'D', double>);
    Pothos::PluginRegistry::add("/proxy/converters/java/jdouble_to_double",
        Pothos::ProxyConvertPair("D", &convertJPrimitiveToNum<'D', double>));
}

/***********************************************************************
 * String
 **********************************************************************/
static Pothos::Proxy convertStringToJString(Pothos::ProxyEnvironment::Sptr env, const std::string &s)
{
    auto jenv = std::dynamic_pointer_cast<JavaProxyEnvironment>(env);
    return jenv->makeHandle(jenv->env->NewStringUTF(s.c_str()));
}

static std::string convertJStringToString(const Pothos::Proxy &proxy)
{
    auto handle = std::dynamic_pointer_cast<JavaProxyHandle>(proxy.getHandle());
    auto env = std::dynamic_pointer_cast<JavaProxyEnvironment>(proxy.getEnvironment());
    return env->jstringToString(handle->value.l);
}

pothos_static_block(pothosRegisterJavaStringConversions)
{
    Pothos::PluginRegistry::addCall("/proxy/converters/java/string_to_jstring",
        &convertStringToJString);
    Pothos::PluginRegistry::add("/proxy/converters/java/jstring_to_string",
        Pothos::ProxyConvertPair("java.lang.String", &convertJStringToString));
}

/***********************************************************************
 * Convert to Java Arrays
 **********************************************************************/
template <char sig, typename T>
Pothos::Proxy convertNumericVectorToJArray(Pothos::ProxyEnvironment::Sptr env, const std::vector<T> &vec)
{
    auto jenv = std::dynamic_pointer_cast<JavaProxyEnvironment>(env);
    switch (sig)
    {
    #define cnvtja(PrimitiveType, jtype) \
    { \
        auto ar = jenv->env->New ## PrimitiveType ## Array(vec.size()); \
        jtype *elems = jenv->env->Get ## PrimitiveType ## ArrayElements(ar, nullptr); \
        for (size_t i = 0; i < vec.size(); i++) elems[i] = static_cast<jtype>(vec[i]); \
        jenv->env->Release ## PrimitiveType ## ArrayElements(ar, elems, 0 /*mode*/); \
        return jenv->makeHandle(ar); \
    }
    case 'Z': cnvtja(Boolean, jboolean)
    case 'C': cnvtja(Char, jchar)
    case 'B': cnvtja(Byte, jbyte)
    case 'S': cnvtja(Short, jshort)
    case 'I': cnvtja(Int, jint)
    case 'J': cnvtja(Long, jlong)
    case 'F': cnvtja(Float, jfloat)
    case 'D': cnvtja(Double, jdouble)
    }
    poco_bugcheck_msg(std::string(1, sig).c_str()); throw;
}

template <char sig, typename T>
std::vector<T> convertJArrayToNumericVector(const Pothos::Proxy &proxy)
{
    auto jenv = std::dynamic_pointer_cast<JavaProxyEnvironment>(proxy.getEnvironment());
    switch (sig)
    {
    #define cjatnv(PrimitiveType, jtype) \
    { \
        auto ar = (jtype ## Array) std::dynamic_pointer_cast<JavaProxyHandle>(proxy.getHandle())->value.l; \
        std::vector<T> vec(jenv->env->GetArrayLength(ar)); \
        jtype *elems = jenv->env->Get ## PrimitiveType ## ArrayElements(ar, nullptr); \
        for (size_t i = 0; i < vec.size(); i++) vec[i] = static_cast<T>(elems[i]); \
        jenv->env->Release ## PrimitiveType ## ArrayElements(ar, elems, JNI_ABORT); \
        return vec; \
    }
    case 'Z': cjatnv(Boolean, jboolean)
    case 'C': cjatnv(Char, jchar)
    case 'B': cjatnv(Byte, jbyte)
    case 'S': cjatnv(Short, jshort)
    case 'I': cjatnv(Int, jint)
    case 'J': cjatnv(Long, jlong)
    case 'F': cjatnv(Float, jfloat)
    case 'D': cjatnv(Double, jdouble)
    }
    poco_bugcheck_msg(std::string(1, sig).c_str()); throw;
}

pothos_static_block(pothosRegisterJavaArrayConversions)
{
    Pothos::PluginRegistry::addCall("/proxy/converters/java/vecbool_to_jbooleanarray",
        &convertNumericVectorToJArray<'Z', bool>);
    Pothos::PluginRegistry::add("/proxy/converters/java/jbooleanarray_to_vecbool",
        Pothos::ProxyConvertPair("[Z", &convertJArrayToNumericVector<'Z', bool>));

    Pothos::PluginRegistry::addCall("/proxy/converters/java/vecchar_to_jchararray",
        &convertNumericVectorToJArray<'C', char>);
    Pothos::PluginRegistry::add("/proxy/converters/java/jchararray_to_vecchar",
        Pothos::ProxyConvertPair("[C", &convertJArrayToNumericVector<'C', char>));

    Pothos::PluginRegistry::addCall("/proxy/converters/java/vecschar_to_jbytearray",
        &convertNumericVectorToJArray<'B', signed char>);
    Pothos::PluginRegistry::addCall("/proxy/converters/java/vecuchar_to_jbytearray",
        &convertNumericVectorToJArray<'B', unsigned char>);
    Pothos::PluginRegistry::add("/proxy/converters/java/jbytearray_to_vecint8",
        Pothos::ProxyConvertPair("[B", &convertJArrayToNumericVector<'B', int8_t>));

    Pothos::PluginRegistry::addCall("/proxy/converters/java/vecsshort_to_jshortarray",
        &convertNumericVectorToJArray<'S', signed short>);
    Pothos::PluginRegistry::addCall("/proxy/converters/java/vecushort_to_jshortarray",
        &convertNumericVectorToJArray<'S', unsigned short>);
    Pothos::PluginRegistry::add("/proxy/converters/java/jshortarray_to_vecint16",
        Pothos::ProxyConvertPair("[S", &convertJArrayToNumericVector<'S', int16_t>));

    Pothos::PluginRegistry::addCall("/proxy/converters/java/vecsint_to_jintarray",
        &convertNumericVectorToJArray<'I', signed int>);
    Pothos::PluginRegistry::addCall("/proxy/converters/java/vecuint_to_jintarray",
        &convertNumericVectorToJArray<'I', unsigned int>);
    Pothos::PluginRegistry::add("/proxy/converters/java/jintarray_to_vecint32",
        Pothos::ProxyConvertPair("[I", &convertJArrayToNumericVector<'I', int32_t>));

    #ifndef POCO_LONG_IS_64_BIT
    Pothos::PluginRegistry::addCall("/proxy/converters/java/vecslong_to_jintarray",
        &convertNumericVectorToJArray<'I', signed long>);
    Pothos::PluginRegistry::addCall("/proxy/converters/java/veculong_to_jintarray",
        &convertNumericVectorToJArray<'I', unsigned long>);
    #else
    Pothos::PluginRegistry::addCall("/proxy/converters/java/vecslong_to_jlongarray",
        &convertNumericVectorToJArray<'J', signed long>);
    Pothos::PluginRegistry::addCall("/proxy/converters/java/veculong_to_jlongarray",
        &convertNumericVectorToJArray<'J', unsigned long>);
    #endif

    Pothos::PluginRegistry::addCall("/proxy/converters/java/vecsllong_to_jlongarray",
        &convertNumericVectorToJArray<'J', signed long long>);
    Pothos::PluginRegistry::addCall("/proxy/converters/java/vecullong_to_jlongarray",
        &convertNumericVectorToJArray<'J', unsigned long long>);
    Pothos::PluginRegistry::add("/proxy/converters/java/jlongarray_to_vecint64",
        Pothos::ProxyConvertPair("[J", &convertJArrayToNumericVector<'J', int64_t>));

    Pothos::PluginRegistry::addCall("/proxy/converters/java/vecfloat_to_jfloatarray",
        &convertNumericVectorToJArray<'F', float>);
    Pothos::PluginRegistry::add("/proxy/converters/java/jfloatarray_to_vecfloat",
        Pothos::ProxyConvertPair("[F", &convertJArrayToNumericVector<'F', float>));

    Pothos::PluginRegistry::addCall("/proxy/converters/java/vecdouble_to_jdoublearray",
        &convertNumericVectorToJArray<'D', double>);
    Pothos::PluginRegistry::add("/proxy/converters/java/jdoublearray_to_vecdouble",
        Pothos::ProxyConvertPair("[D", &convertJArrayToNumericVector<'D', double>));
}

/***********************************************************************
 * Object Arrays
 **********************************************************************/
/*
static Pothos::Proxy convertVectorToJObjectArray(Pothos::ProxyEnvironment::Sptr env, const Pothos::ProxyVector &vec)
{
    auto jenv = std::dynamic_pointer_cast<JavaProxyEnvironment>(env);
    auto ar = (jobjectArray) jenv->env->NewObjectArray(vec.size(), jenv->FindClass("java/lang/Object"), nullptr);
    for (size_t i = 0; i < vec.size(); i++)
    {
        jenv->env->SetObjectArrayElement(ar, i, jenv->getHandle(vec[i])->toJobject());
    }
    return jenv->makeHandle(ar);
}
*/

static Pothos::ProxyVector convertJObjectArrayToVector(const Pothos::Proxy &proxy)
{
    auto handle = std::dynamic_pointer_cast<JavaProxyHandle>(proxy.getHandle());
    auto jenv = std::dynamic_pointer_cast<JavaProxyEnvironment>(proxy.getEnvironment());

    auto ar = (jobjectArray) handle->value.l;
    Pothos::ProxyVector vec(jenv->env->GetArrayLength(ar));
    for (size_t i = 0; i < vec.size(); i++)
    {
        vec[i] = jenv->makeHandle(jenv->env->GetObjectArrayElement(ar, i));
    }
    return vec;
}

pothos_static_block(pothosRegisterJavaObjectArrayConversions)
{
    //Pothos::PluginRegistry::addCall("/proxy/converters/java/vector_to_jarray",
    //    &convertVectorToJObjectArray);
    Pothos::PluginRegistry::add("/proxy/converters/java/jarray_to_vector",
        Pothos::ProxyConvertPair("[Ljava.lang.Object;", &convertJObjectArrayToVector));
}

/***********************************************************************
 * Primitive Object representations
 **********************************************************************/
template <typename T>
static T javaPrimitiveObjectToType(
    const std::string &name,
    const Pothos::Proxy &proxy)
{
    return proxy.call<T>(name);
}

pothos_static_block(pothosRegisterJavaDefaultConversions)
{
    Pothos::PluginRegistry::add("/proxy/converters/java/javalangboolean_to_bool",
        Pothos::ProxyConvertPair("java.lang.Boolean", Pothos::Callable(
            &javaPrimitiveObjectToType<bool>).bind("booleanValue", 0)));

    Pothos::PluginRegistry::add("/proxy/converters/java/javalangcharacter_to_char",
        Pothos::ProxyConvertPair("java.lang.Character", Pothos::Callable(
            &javaPrimitiveObjectToType<char>).bind("charValue", 0)));

    Pothos::PluginRegistry::add("/proxy/converters/java/javalangbyte_to_signed_char",
        Pothos::ProxyConvertPair("java.lang.Byte", Pothos::Callable(
            &javaPrimitiveObjectToType<signed char>).bind("byteValue", 0)));

    Pothos::PluginRegistry::add("/proxy/converters/java/javalangshort_to_short",
        Pothos::ProxyConvertPair("java.lang.Short", Pothos::Callable(
            &javaPrimitiveObjectToType<signed short>).bind("shortValue", 0)));

    Pothos::PluginRegistry::add("/proxy/converters/java/javalanginteger_to_int",
        Pothos::ProxyConvertPair("java.lang.Integer", Pothos::Callable(
            &javaPrimitiveObjectToType<signed int>).bind("intValue", 0)));

    Pothos::PluginRegistry::add("/proxy/converters/java/javalanglong_to_long_long",
        Pothos::ProxyConvertPair("java.lang.Long", Pothos::Callable(
            &javaPrimitiveObjectToType<signed long long>).bind("longValue", 0)));

    Pothos::PluginRegistry::add("/proxy/converters/java/javalangfloat_to_float",
        Pothos::ProxyConvertPair("java.lang.Float", Pothos::Callable(
            &javaPrimitiveObjectToType<float>).bind("floatValue", 0)));

    Pothos::PluginRegistry::add("/proxy/converters/java/javalangdouble_to_double",
        Pothos::ProxyConvertPair("java.lang.Double", Pothos::Callable(
            &javaPrimitiveObjectToType<double>).bind("doubleValue", 0)));
}
