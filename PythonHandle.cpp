// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Poco/Format.h>
#include <cassert>
#include <iostream>
#include "PythonProxy.hpp"

PythonProxyHandle::PythonProxyHandle(std::shared_ptr<PythonProxyEnvironment> env, PyObject *obj, const bool borrowed):
    env(env), obj(obj), ref(PyObjectRef(obj, borrowed))
{
    return;
}

PythonProxyHandle::~PythonProxyHandle(void)
{
    PyGilStateLock lock;
    ref = PyObjectRef();
}

int PythonProxyHandle::compareTo(const Pothos::Proxy &proxy) const
{
    PyGilStateLock lock;
    int rEq = 0, rGt = 0, rLt = 0;
    rEq = PyObject_RichCompareBool(obj, env->getHandle(proxy)->obj, Py_EQ);
    if (rEq == 1) return 0;
    if (rEq == -1) goto fail;
    rGt = PyObject_RichCompareBool(obj, env->getHandle(proxy)->obj, Py_GT);
    if (rGt == 1) return +1;
    if (rGt == -1) goto fail;
    rLt = PyObject_RichCompareBool(obj, env->getHandle(proxy)->obj, Py_LT);
    if (rLt == 1) return -1;
    if (rLt == -1) goto fail;
    fail:
    const int r = 0;
    auto errorMsg = getErrorString();
    if (not errorMsg.empty())
    {
        throw Pothos::ProxyCompareError("PythonProxyHandle::compareTo()", errorMsg);
    }
    return r;
}

size_t PythonProxyHandle::hashCode(void) const
{
    PyGilStateLock lock;
    return size_t(PyObject_Hash(obj));
}

std::string PythonProxyHandle::toString(void) const
{
    PyGilStateLock lock;
    PyObjectRef str(PyObject_Str(obj), REF_NEW);
    return PyObjToStdString(str.obj);
}

std::string PythonProxyHandle::getClassName(void) const
{
    PyGilStateLock lock;
    PyObjectRef type(PyObject_Type(obj), REF_NEW);
    PyObjectRef name(PyObject_GetAttrString(type.obj, "__name__"), REF_NEW);
    return PyObjToStdString(name.obj);
}

Pothos::Proxy PythonProxyHandle::call(const std::string &name, const Pothos::Proxy *args, const size_t numArgs)
{
    PyGilStateLock lock;
    if (this->obj == nullptr) throw Pothos::ProxyHandleCallError(
        "PythonProxyHandle::call("+name+")", "cant call on a null object");

    /*******************************************************************
     * Step 1) locate the callable object
     ******************************************************************/
    PyObjectRef attrObj;

    if (name.empty()) attrObj = PyObjectRef(ref);
    else attrObj = PyObjectRef(PyObject_GetAttrString(this->obj, name.c_str()), REF_NEW);

    if (attrObj.obj == nullptr)
    {
        throw Pothos::ProxyHandleCallError(
            "PythonProxyHandle::call("+name+")",
            Poco::format("no attribute on %s", this->toString()));
    }

    if (not PyCallable_Check(attrObj.obj))
    {
        if (numArgs == 0) return env->makeHandle(attrObj); //access a field
        else throw Pothos::ProxyHandleCallError(
            "PythonProxyHandle::call("+name+")",
            Poco::format("cant call on %s", this->toString()));
    }

    /*******************************************************************
     * Step 2) create tuple of arguments
     ******************************************************************/
    PyObjectRef argsObj(PyTuple_New(numArgs), REF_NEW);
    std::vector<std::shared_ptr<PythonProxyHandle>> argHandles(numArgs);
    for (size_t i = 0; i < numArgs; i++)
    {
        argHandles[i] = env->getHandle(args[i]);
        PyTuple_SetItem(argsObj.obj, i, argHandles[i]->ref.newRef());
    }

    /*******************************************************************
     * Step 4) call into the callable object
     ******************************************************************/
    PyObjectRef result(PyObject_CallObject(attrObj.obj, argsObj.obj), REF_NEW);

    /*******************************************************************
     * Step 5) exception handling and reporting
     ******************************************************************/
    auto errorMsg = getErrorString();
    if (not errorMsg.empty())
    {
        throw Pothos::ProxyExceptionMessage(errorMsg);
    }

    return env->makeHandle(result);
}
