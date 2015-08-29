// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Python.h>
#include <Pothos/Proxy.hpp>
#include <functional>
#include <iostream>

/***********************************************************************
 * Conversion function pointer types
 **********************************************************************/
typedef std::function<Pothos::Proxy(Pothos::ProxyEnvironment::Sptr, PyObject*)> PyObjectToProxyFcn;
typedef std::function<PyObject *(const Pothos::Proxy &)> ProxyToPyObjectFcn;

/***********************************************************************
 * simple holder of a object ref
 **********************************************************************/
#define REF_BORROWED true
#define REF_NEW false

struct PyObjectRef
{
    PyObjectRef(void):
        obj(nullptr)
    {
        return;
    }

    PyObjectRef(const PyObjectRef &ref):
        obj(ref.obj)
    {
        Py_XINCREF(obj);
    }

    PyObjectRef &operator=(const PyObjectRef &ref)
    {
        Py_XDECREF(obj);
        obj = ref.obj;
        Py_XINCREF(obj);
        return *this;
    }

    PyObjectRef(PyObject *obj, const bool borrowed):
        obj(obj)
    {
        if (borrowed) Py_XINCREF(obj);
    }

    ~PyObjectRef(void)
    {
        Py_XDECREF(obj);
    }

    PyObject *newRef(void)
    {
        Py_XINCREF(obj);
        return obj;
    }

    PyObject *obj;
};

/***********************************************************************
 * C++ locking structures for calling into and out of the interpreter
 **********************************************************************/
struct PyGilStateLock
{
    PyGILState_STATE _s;
    PyGilStateLock(void):_s(PyGILState_Ensure()){}
    ~PyGilStateLock(void){PyGILState_Release(_s);}
};

struct PyThreadStateLock
{
    PyThreadState *_s;
    PyThreadStateLock(void):_s(PyEval_SaveThread()){}
    ~PyThreadStateLock(void){PyEval_RestoreThread(_s);}
};
