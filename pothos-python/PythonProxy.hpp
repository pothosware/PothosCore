// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include "PyObjectUtils.hpp"
#include <Pothos/Config.hpp>
#include <Pothos/Proxy.hpp>
#include <Pothos/Callable.hpp>
#include <string>

class PythonProxyHandle;

inline std::string PyObjToStdString(PyObject *o)
{
    #if PY_MAJOR_VERSION >= 3
    assert(PyUnicode_Check(o));
    Py_ssize_t size = 0;
    const char *c = PyUnicode_AsUTF8AndSize(o, &size);
    return std::string(c, size);
    #else
    assert(PyString_Check(o));
    return std::string(PyString_AsString(o), PyString_Size(o));
    #endif
}

inline PyObject *StdStringToPyObject(const std::string &s)
{
    #if PY_MAJOR_VERSION >= 3
    return PyUnicode_DecodeLocaleAndSize(s.c_str(), s.size(), nullptr);
    #else
    return PyString_FromStringAndSize(s.c_str(), s.size());
    #endif
}

inline std::string getErrorString(void)
{
    if (not PyErr_Occurred()) return "";
    PyObject *type = nullptr, *value = nullptr, *traceback = nullptr;
    PyErr_Fetch(&type, &value, &traceback);
    assert(value != nullptr);
    std::string errorMsg = PyObjToStdString(PyObjectRef(PyObject_Str(value), REF_NEW).obj);
    Py_XDECREF(type);
    Py_XDECREF(value);
    Py_XDECREF(traceback);
    PyErr_Clear();
    return errorMsg;
}

/***********************************************************************
 * custom Python environment overload
 **********************************************************************/
class PythonProxyEnvironment :
    public Pothos::ProxyEnvironment
{
public:
    PythonProxyEnvironment(const Pothos::ProxyEnvironmentArgs &);

    Pothos::Proxy makeHandle(PyObject *obj, const bool borrowed);
    Pothos::Proxy makeHandle(const PyObjectRef &ref);

    std::shared_ptr<PythonProxyHandle> getHandle(const Pothos::Proxy &proxy);

    std::string getName(void) const
    {
        return "python";
    }

    Pothos::Proxy findProxy(const std::string &name);

    Pothos::Proxy convertObjectToProxy(const Pothos::Object &local);
    Pothos::Object convertProxyToObject(const Pothos::Proxy &proxy);
};

/***********************************************************************
 * custom Python class handler overload
 **********************************************************************/
class PythonProxyHandle : public Pothos::ProxyHandle
{
public:

    PythonProxyHandle(std::shared_ptr<PythonProxyEnvironment> env, PyObject *obj, const bool borrowed);

    ~PythonProxyHandle(void);

    Pothos::ProxyEnvironment::Sptr getEnvironment(void) const
    {
        return env;
    }

    Pothos::Proxy call(const std::string &name, const Pothos::Proxy *args, const size_t numArgs);
    int compareTo(const Pothos::Proxy &proxy) const;
    size_t hashCode(void) const;
    std::string toString(void) const;
    std::string getClassName(void) const;

    std::shared_ptr<PythonProxyEnvironment> env;

    PyObject *obj;
    PyObjectRef ref;
};
