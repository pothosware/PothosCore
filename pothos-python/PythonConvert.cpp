// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Plugin.hpp>
#include <cassert>
#include <complex>
#include <iostream>
#include "PythonProxy.hpp"

/***********************************************************************
 * None
 **********************************************************************/
static Pothos::Proxy convertNullToPyNone(Pothos::ProxyEnvironment::Sptr env, const Pothos::NullObject &)
{
    return std::dynamic_pointer_cast<PythonProxyEnvironment>(env)->makeHandle(Py_None, REF_BORROWED);
}

static Pothos::NullObject convertPyNoneToNull(const Pothos::Proxy &)
{
    return Pothos::NullObject();
}

pothos_static_block(pothosRegisterPythonNullConversions)
{
    Pothos::PluginRegistry::addCall("/proxy/converters/python/null_to_pynone",
        &convertNullToPyNone);
    Pothos::PluginRegistry::add("/proxy/converters/python/pynone_to_null",
        Pothos::ProxyConvertPair("NoneType", &convertPyNoneToNull));
}

/***********************************************************************
 * bool
 **********************************************************************/
static Pothos::Proxy convertBoolToPyBool(Pothos::ProxyEnvironment::Sptr env, const bool &b)
{
    return std::dynamic_pointer_cast<PythonProxyEnvironment>(env)->makeHandle(PyBool_FromLong(b), REF_NEW);
}

static bool convertPyBoolToBool(const Pothos::Proxy &proxy)
{
    return bool(std::dynamic_pointer_cast<PythonProxyHandle>(proxy.getHandle())->obj == Py_True);
}

pothos_static_block(pothosRegisterPythonBoolConversions)
{
    Pothos::PluginRegistry::addCall("/proxy/converters/python/bool_to_pybool",
        &convertBoolToPyBool);
    Pothos::PluginRegistry::add("/proxy/converters/python/pybool_to_bool",
        Pothos::ProxyConvertPair("bool", &convertPyBoolToBool));
}

/***********************************************************************
 * integer types
 **********************************************************************/
template <typename T>
static Pothos::Proxy convertIntNumToPyInt(Pothos::ProxyEnvironment::Sptr env, const T &num)
{
    #if PY_MAJOR_VERSION >= 3
    return std::dynamic_pointer_cast<PythonProxyEnvironment>(env)->makeHandle(PyLong_FromLong(long(num)), REF_NEW);
    #else
    return std::dynamic_pointer_cast<PythonProxyEnvironment>(env)->makeHandle(PyInt_FromLong(long(num)), REF_NEW);
    #endif
}

static long convertPyIntToLong(const Pothos::Proxy &proxy)
{
    #if PY_MAJOR_VERSION >= 3
    return PyLong_AsLong(std::dynamic_pointer_cast<PythonProxyHandle>(proxy.getHandle())->obj);
    #else
    return PyInt_AsLong(std::dynamic_pointer_cast<PythonProxyHandle>(proxy.getHandle())->obj);
    #endif
}

pothos_static_block(pothosRegisterPythonIntConversions)
{
    Pothos::PluginRegistry::addCall("/proxy/converters/python/char_to_pyint",
        &convertIntNumToPyInt<char>);
    Pothos::PluginRegistry::addCall("/proxy/converters/python/schar_to_pyint",
        &convertIntNumToPyInt<signed char>);
    Pothos::PluginRegistry::addCall("/proxy/converters/python/uchar_to_pyint",
        &convertIntNumToPyInt<unsigned char>);

    Pothos::PluginRegistry::addCall("/proxy/converters/python/sshort_to_pyint",
        &convertIntNumToPyInt<signed short>);
    Pothos::PluginRegistry::addCall("/proxy/converters/python/ushort_to_pyint",
        &convertIntNumToPyInt<unsigned short>);

    Pothos::PluginRegistry::addCall("/proxy/converters/python/sint_to_pyint",
        &convertIntNumToPyInt<signed int>);
    Pothos::PluginRegistry::addCall("/proxy/converters/python/uint_to_pyint",
        &convertIntNumToPyInt<unsigned int>);

    Pothos::PluginRegistry::addCall("/proxy/converters/python/slong_to_pyint",
        &convertIntNumToPyInt<signed long>);
    Pothos::PluginRegistry::addCall("/proxy/converters/python/ulong_to_pyint",
        &convertIntNumToPyInt<unsigned long>);

    Pothos::PluginRegistry::add("/proxy/converters/python/pyint_to_long",
        Pothos::ProxyConvertPair("int", &convertPyIntToLong));
}

/***********************************************************************
 * long types
 **********************************************************************/
template <typename T>
static Pothos::Proxy convertLongLongToPyLong(Pothos::ProxyEnvironment::Sptr env, const T &num)
{
    return std::dynamic_pointer_cast<PythonProxyEnvironment>(env)->makeHandle(PyLong_FromLongLong((long long)(num)), REF_NEW);
}

static long long convertPyLongToLongLong(const Pothos::Proxy &proxy)
{
    return PyLong_AsLongLong(std::dynamic_pointer_cast<PythonProxyHandle>(proxy.getHandle())->obj);
}

pothos_static_block(pothosRegisterPythonLongConversions)
{
    Pothos::PluginRegistry::addCall("/proxy/converters/python/sllong_to_pylong",
        &convertLongLongToPyLong<signed long long>);
    Pothos::PluginRegistry::addCall("/proxy/converters/python/ullong_to_pylong",
        &convertLongLongToPyLong<unsigned long long>);

    Pothos::PluginRegistry::add("/proxy/converters/python/pylong_to_llong",
        Pothos::ProxyConvertPair("long", &convertPyLongToLongLong));
}

/***********************************************************************
 * float types
 **********************************************************************/
template <typename T>
static Pothos::Proxy convertFloatNumToPyFloat(Pothos::ProxyEnvironment::Sptr env, const T &num)
{
    return std::dynamic_pointer_cast<PythonProxyEnvironment>(env)->makeHandle(PyFloat_FromDouble(double(num)), REF_NEW);
}

static double convertPyFloatToDouble(const Pothos::Proxy &proxy)
{
    return PyFloat_AsDouble(std::dynamic_pointer_cast<PythonProxyHandle>(proxy.getHandle())->obj);
}

pothos_static_block(pothosRegisterPythonFloatConversions)
{
    Pothos::PluginRegistry::addCall("/proxy/converters/python/float_to_pyfloat",
        &convertFloatNumToPyFloat<float>);
    Pothos::PluginRegistry::addCall("/proxy/converters/python/double_to_pyfloat",
        &convertFloatNumToPyFloat<double>);

    Pothos::PluginRegistry::add("/proxy/converters/python/pyfloat_to_double",
        Pothos::ProxyConvertPair("float", &convertPyFloatToDouble));
}

/***********************************************************************
 * complex types
 **********************************************************************/
template <typename T>
static Pothos::Proxy convertComplexToPyComplex(Pothos::ProxyEnvironment::Sptr env, const std::complex<T> &c)
{
    return std::dynamic_pointer_cast<PythonProxyEnvironment>(env)->makeHandle(PyComplex_FromDoubles(double(c.real()), double(c.imag())), REF_NEW);
}

static std::complex<double> convertPyComplexToComplex(const Pothos::Proxy &proxy)
{
    auto c =  PyComplex_AsCComplex(std::dynamic_pointer_cast<PythonProxyHandle>(proxy.getHandle())->obj);
    return std::complex<double>(c.real, c.imag);
}

pothos_static_block(pothosRegisterPythonComplexConversions)
{
    Pothos::PluginRegistry::addCall("/proxy/converters/python/complex64_to_pycomplex",
        &convertComplexToPyComplex<float>);
    Pothos::PluginRegistry::addCall("/proxy/converters/python/complex128_to_pycomplex",
        &convertComplexToPyComplex<double>);

    Pothos::PluginRegistry::add("/proxy/converters/python/pycomplex_to_complex128",
        Pothos::ProxyConvertPair("complex", &convertPyComplexToComplex));
}

/***********************************************************************
 * string
 **********************************************************************/
static Pothos::Proxy convertStringToPyString(Pothos::ProxyEnvironment::Sptr env, const std::string &s)
{
    return std::dynamic_pointer_cast<PythonProxyEnvironment>(env)->makeHandle(StdStringToPyObject(s), REF_NEW);
}

static std::string convertPyStringToString(const Pothos::Proxy &proxy)
{
    auto obj = std::dynamic_pointer_cast<PythonProxyHandle>(proxy.getHandle())->obj;
    return PyObjToStdString(obj);
}

static Pothos::Proxy convertStrVecToPyList(Pothos::ProxyEnvironment::Sptr env, const std::vector<std::string> &strings)
{
    Pothos::ProxyVector vec;
    for (const auto &s : strings) vec.push_back(env->makeProxy(s));
    return env->makeProxy(vec);
}

pothos_static_block(pothosRegisterPythonStringConversions)
{
    Pothos::PluginRegistry::addCall("/proxy/converters/python/string_to_pystring",
        &convertStringToPyString);
    Pothos::PluginRegistry::add("/proxy/converters/python/pystring_to_string",
        Pothos::ProxyConvertPair("str", &convertPyStringToString));
    Pothos::PluginRegistry::addCall("/proxy/converters/python/vecstring_to_pylist",
        &convertStrVecToPyList);
}

/***********************************************************************
 * bytes
 **********************************************************************/
template <typename ByteType>
Pothos::Proxy convertByteVectorToPyBytes(Pothos::ProxyEnvironment::Sptr env, const std::vector<ByteType> &vec)
{
    auto o = PyBytes_FromStringAndSize(reinterpret_cast<const char *>(vec.data()), vec.size());
    return std::dynamic_pointer_cast<PythonProxyEnvironment>(env)->makeHandle(o, REF_NEW);
}

static std::vector<char> convertPyBytesToString(const Pothos::Proxy &proxy)
{
    auto obj = std::dynamic_pointer_cast<PythonProxyHandle>(proxy.getHandle())->obj;
    auto c = PyBytes_AsString(obj);
    return std::vector<char>(c, c+PyBytes_Size(obj));
}

pothos_static_block(pothosRegisterPythonBytesConversions)
{
    Pothos::PluginRegistry::addCall("/proxy/converters/python/vecchar_to_pybytes",
        &convertByteVectorToPyBytes<char>);
    Pothos::PluginRegistry::addCall("/proxy/converters/python/vecschar_to_pybytes",
        &convertByteVectorToPyBytes<signed char>);
    Pothos::PluginRegistry::addCall("/proxy/converters/python/vecuchar_to_pybytes",
        &convertByteVectorToPyBytes<unsigned char>);
    Pothos::PluginRegistry::add("/proxy/converters/python/pybytes_to_string",
        Pothos::ProxyConvertPair("bytes", &convertPyBytesToString));
}

/***********************************************************************
 * numeric
 **********************************************************************/
template <typename T>
static Pothos::Proxy convertIntVectorToPyList(Pothos::ProxyEnvironment::Sptr env, const std::vector<T> &vec)
{
    auto pyenv = std::dynamic_pointer_cast<PythonProxyEnvironment>(env);
    PyObjectRef pyList(PyList_New(vec.size()), REF_NEW);
    for (size_t i = 0; i < vec.size(); i++)
    {
        #if PY_MAJOR_VERSION >= 3
        PyList_SetItem(pyList.obj, i, PyLong_FromLong(long(vec[i])));
        #else
        PyList_SetItem(pyList.obj, i, PyInt_FromLong(long(vec[i])));
        #endif
    }
    return pyenv->makeHandle(pyList);
}

template <typename T>
static Pothos::Proxy convertLongLongVectorToPyList(Pothos::ProxyEnvironment::Sptr env, const std::vector<T> &vec)
{
    auto pyenv = std::dynamic_pointer_cast<PythonProxyEnvironment>(env);
    PyObjectRef pyList(PyList_New(vec.size()), REF_NEW);
    for (size_t i = 0; i < vec.size(); i++)
    {
        PyList_SetItem(pyList.obj, i, PyLong_FromLongLong((long long)(vec[i])));
    }
    return pyenv->makeHandle(pyList);
}

template <typename T>
static Pothos::Proxy convertFloatVectorToPyList(Pothos::ProxyEnvironment::Sptr env, const std::vector<T> &vec)
{
    auto pyenv = std::dynamic_pointer_cast<PythonProxyEnvironment>(env);
    PyObjectRef pyList(PyList_New(vec.size()), REF_NEW);
    for (size_t i = 0; i < vec.size(); i++)
    {
        PyList_SetItem(pyList.obj, i, PyFloat_FromDouble(double(vec[i])));
    }
    return pyenv->makeHandle(pyList);
}

template <typename T>
static Pothos::Proxy convertComplexVectorToPyList(Pothos::ProxyEnvironment::Sptr env, const std::vector<std::complex<T>> &vec)
{
    auto pyenv = std::dynamic_pointer_cast<PythonProxyEnvironment>(env);
    PyObjectRef pyList(PyList_New(vec.size()), REF_NEW);
    for (size_t i = 0; i < vec.size(); i++)
    {
        PyList_SetItem(pyList.obj, i, PyComplex_FromDoubles(double(vec[i].real()), double(vec[i].imag())));
    }
    return pyenv->makeHandle(pyList);
}

pothos_static_block(pothosRegisterPythonNumericConversions)
{
    Pothos::PluginRegistry::addCall("/proxy/converters/python/vecsshort_to_pylist",
        &convertIntVectorToPyList<signed short>);
    Pothos::PluginRegistry::addCall("/proxy/converters/python/vecushort_to_pylist",
        &convertIntVectorToPyList<unsigned short>);
    Pothos::PluginRegistry::addCall("/proxy/converters/python/vecsint_to_pylist",
        &convertIntVectorToPyList<signed int>);
    Pothos::PluginRegistry::addCall("/proxy/converters/python/vecuint_to_pylist",
        &convertIntVectorToPyList<unsigned int>);
    Pothos::PluginRegistry::addCall("/proxy/converters/python/vecslong_to_pylist",
        &convertIntVectorToPyList<signed long>);
    Pothos::PluginRegistry::addCall("/proxy/converters/python/veculong_to_pylist",
        &convertIntVectorToPyList<unsigned long>);
    Pothos::PluginRegistry::addCall("/proxy/converters/python/vecsllong_to_pylist",
        &convertLongLongVectorToPyList<signed long long>);
    Pothos::PluginRegistry::addCall("/proxy/converters/python/vecullong_to_pylist",
        &convertLongLongVectorToPyList<unsigned long long>);
    Pothos::PluginRegistry::addCall("/proxy/converters/python/vecfloat_to_pylist",
        &convertFloatVectorToPyList<float>);
    Pothos::PluginRegistry::addCall("/proxy/converters/python/vecdouble_to_pylist",
        &convertFloatVectorToPyList<double>);
    Pothos::PluginRegistry::addCall("/proxy/converters/python/veccomplexfloat_to_pylist",
        &convertComplexVectorToPyList<float>);
    Pothos::PluginRegistry::addCall("/proxy/converters/python/veccomplexdouble_to_pylist",
        &convertComplexVectorToPyList<double>);
}

/***********************************************************************
 * tuple
 **********************************************************************/
static Pothos::ProxyVector convertPyTupleToVector(const Pothos::Proxy &proxy)
{
    auto env = std::dynamic_pointer_cast<PythonProxyEnvironment>(proxy.getEnvironment());
    auto obj = std::dynamic_pointer_cast<PythonProxyHandle>(proxy.getHandle())->obj;
    Pothos::ProxyVector vec(PyTuple_Size(obj));
    for (size_t i = 0; i < vec.size(); i++)
    {
        vec[i] = env->makeHandle(PyTuple_GetItem(obj, i), REF_BORROWED);
    }
    return vec;
}

pothos_static_block(pothosRegisterPythonTupleConversions)
{
    Pothos::PluginRegistry::add("/proxy/converters/python/pytuple_to_vector",
        Pothos::ProxyConvertPair("tuple", &convertPyTupleToVector));
}

/***********************************************************************
 * list
 **********************************************************************/
static Pothos::Proxy convertVectorToPyList(Pothos::ProxyEnvironment::Sptr env, const Pothos::ProxyVector &vec)
{
    auto pyenv = std::dynamic_pointer_cast<PythonProxyEnvironment>(env);
    PyObjectRef pyList(PyList_New(vec.size()), REF_NEW);
    for (size_t i = 0; i < vec.size(); i++)
    {
        PyList_SetItem(pyList.obj, i, pyenv->getHandle(vec[i])->ref.newRef());
    }
    return pyenv->makeHandle(pyList);
}

static Pothos::ProxyVector convertPyListToVector(const Pothos::Proxy &proxy)
{
    auto env = std::dynamic_pointer_cast<PythonProxyEnvironment>(proxy.getEnvironment());
    auto obj = std::dynamic_pointer_cast<PythonProxyHandle>(proxy.getHandle())->obj;
    Pothos::ProxyVector vec(PyList_Size(obj));
    for (size_t i = 0; i < vec.size(); i++)
    {
        vec[i] = env->makeHandle(PyList_GetItem(obj, i), REF_BORROWED);
    }
    return vec;
}

pothos_static_block(pothosRegisterPythonListConversions)
{
    Pothos::PluginRegistry::addCall("/proxy/converters/python/vector_to_pylist",
        &convertVectorToPyList);
    Pothos::PluginRegistry::add("/proxy/converters/python/pylist_to_vector",
        Pothos::ProxyConvertPair("list", &convertPyListToVector));
}

/***********************************************************************
 * set
 **********************************************************************/
static Pothos::Proxy convertSetToPySet(Pothos::ProxyEnvironment::Sptr env, const Pothos::ProxySet &set)
{
    auto pyenv = std::dynamic_pointer_cast<PythonProxyEnvironment>(env);
    PyObjectRef pySet(PySet_New(nullptr), REF_NEW);
    for (const auto &entry : set)
    {
        auto obj = pyenv->getHandle(entry)->obj;
        PySet_Add(pySet.obj, obj);
    }
    return pyenv->makeHandle(pySet);
}

static Pothos::ProxySet convertPySetToSet(const Pothos::Proxy &proxy)
{
    auto env = std::dynamic_pointer_cast<PythonProxyEnvironment>(proxy.getEnvironment());
    auto obj = std::dynamic_pointer_cast<PythonProxyHandle>(proxy.getHandle())->obj;
    auto pySet = PySet_New(obj); //makes a copy which we pop from until empty
    Pothos::ProxySet set;
    while (PySet_Size(pySet) != 0)
    {
        set.insert(env->makeHandle(PySet_Pop(pySet), REF_NEW));
    }
    return set;
}

pothos_static_block(pothosRegisterPythonSetConversions)
{
    Pothos::PluginRegistry::addCall("/proxy/converters/python/set_to_pyset",
        &convertSetToPySet);
    Pothos::PluginRegistry::add("/proxy/converters/python/pyset_to_set",
        Pothos::ProxyConvertPair("set", &convertPySetToSet));
}

/***********************************************************************
 * dict
 **********************************************************************/
static Pothos::Proxy convertMapToPyDict(Pothos::ProxyEnvironment::Sptr env, const Pothos::ProxyMap &d)
{
    auto pyenv = std::dynamic_pointer_cast<PythonProxyEnvironment>(env);
    PyObjectRef pyDict(PyDict_New(), REF_NEW);
    for (const auto &entry : d) PyDict_SetItem(
        pyDict.obj,
        pyenv->getHandle(entry.first.getHandle())->obj,
        pyenv->getHandle(entry.second.getHandle())->obj);
    return pyenv->makeHandle(pyDict);
}

static Pothos::ProxyMap convertPyDictToMap(const Pothos::Proxy &proxy)
{
    auto env = std::dynamic_pointer_cast<PythonProxyEnvironment>(proxy.getEnvironment());
    auto obj = std::dynamic_pointer_cast<PythonProxyHandle>(proxy.getHandle())->obj;
    Pothos::ProxyMap d;
    PyObjectRef items(PyDict_Items(obj), REF_NEW);
    for (Py_ssize_t i = 0; i < PyList_Size(items.obj); i++)
    {
        PyObject *item = PyList_GetItem(items.obj, i);
        assert(PyTuple_Check(item) and PyTuple_Size(item) == 2);
        auto key = env->makeHandle(PyTuple_GetItem(item, 0), REF_BORROWED);
        auto val = env->makeHandle(PyTuple_GetItem(item, 1), REF_BORROWED);
        d[key] = val;
    }
    return d;
}

pothos_static_block(pothosRegisterPythonDictConversions)
{
    Pothos::PluginRegistry::addCall("/proxy/converters/python/map_to_pydict",
        &convertMapToPyDict);
    Pothos::PluginRegistry::add("/proxy/converters/python/pydict_to_map",
        Pothos::ProxyConvertPair("dict", &convertPyDictToMap));
}
