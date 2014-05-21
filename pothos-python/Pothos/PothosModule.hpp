// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "../PyObjectUtils.hpp"
#include <Pothos/Proxy.hpp>

//! Module utility to convert between forms
Pothos::Proxy PyObjectToProxy(PyObject *obj);

//! Module utility to convert between forms
PyObject *ProxyToPyObject(const Pothos::Proxy &proxy);

//! Access the proxy environment for python
Pothos::ProxyEnvironment::Sptr getPythonProxyEnv(void);

//! Convert a proxy from one env into another
inline Pothos::Proxy proxyEnvTranslate(const Pothos::Proxy &proxy, const Pothos::ProxyEnvironment::Sptr &env)
{
    if (proxy.getEnvironment() == env) return proxy;
    auto local = proxy.getEnvironment()->convertProxyToObject(proxy);
    return env->convertObjectToProxy(local);
}

void registerPothosModuleConverters(void);

/***********************************************************************
 * Pothos::ProxyEnvironment support
 **********************************************************************/
struct ProxyEnvironmentObject
{
    PyObject_HEAD
    Pothos::ProxyEnvironment::Sptr *env;
};

//! called by module to register type
void registerProxyEnvironmentType(PyObject *m);

//! utility for c api to construct a proxy env
PyObject *makeProxyEnvironmentObject(const Pothos::ProxyEnvironment::Sptr &env);

//! utility for c api to check if a proxy env
bool isProxyEnvironmentObject(PyObject *obj);

/***********************************************************************
 * Pothos::Proxy support
 **********************************************************************/
struct ProxyObject
{
    PyObject_HEAD
    Pothos::Proxy *proxy;
};

//! called by module to register type
void registerProxyType(PyObject *m);

//! utility for c api to construct a proxy
PyObject *makeProxyObject(const Pothos::Proxy &proxy);

//! utility for c api to check if a proxy
bool isProxyObject(PyObject *obj);

//! Unwrap an already proxied object -- otherwise call PyObjectToProxy()
inline Pothos::Proxy PyObjectToProxyInspect(PyObject *obj)
{
    assert(obj != nullptr);
    if (isProxyObject(obj)) return *reinterpret_cast<ProxyObject *>(obj)->proxy;
    return PyObjectToProxy(obj);
}

/***********************************************************************
 * Pothos::ProxyCall support
 **********************************************************************/
struct ProxyCallObject
{
    PyObject_HEAD
    PyObjectRef *proxy;
    PyObjectRef *name;
};

//! called by module to register type
void registerProxyCallType(PyObject *m);

//! utility for c api to construct a proxy call object
PyObject *makeProxyCallObject(PyObject *args);

/***********************************************************************
 * rich compare support for old-style cmp
 **********************************************************************/
inline PyObject *richCompareFromSimple(const int cmp, const int opid)
{
    switch(opid)
    {
    case Py_LT: return PyBool_FromLong(cmp < 0);
    case Py_LE: return PyBool_FromLong(cmp <= 0);
    case Py_EQ: return PyBool_FromLong(cmp == 0);
    case Py_NE: return PyBool_FromLong(cmp != 0);
    case Py_GT: return PyBool_FromLong(cmp > 0);
    case Py_GE: return PyBool_FromLong(cmp >= 0);
    default: return PyBool_FromLong(0);
    }
}
