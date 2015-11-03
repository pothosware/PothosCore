// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PothosModule.hpp"
#include <Pothos/Plugin.hpp>
#include <Pothos/Init.hpp>
#include <iostream>
#include <cassert>

/***********************************************************************
 * module utility converters
 **********************************************************************/
static Pothos::ProxyEnvironment::Sptr myPythonProxyEnv;
static PyObjectToProxyFcn myPyObjectToProxyFcn;
static ProxyToPyObjectFcn myProxyToPyObjectFcn;

static void initPyObjectUtilityConverters(void)
{
    try
    {
        Pothos::init(); //init here in case python is the caller
        myPythonProxyEnv = Pothos::ProxyEnvironment::make("python");
        myPyObjectToProxyFcn = Pothos::PluginRegistry::get("/proxy_helpers/python/pyobject_to_proxy").getObject().extract<PyObjectToProxyFcn>();
        myProxyToPyObjectFcn = Pothos::PluginRegistry::get("/proxy_helpers/python/proxy_to_pyobject").getObject().extract<ProxyToPyObjectFcn>();
        registerPothosModuleConverters();
    }
    catch (const Pothos::Exception &ex)
    {
        std::cerr << "PothosModule init error: " << ex.displayText() << std::endl;
    }
}

static void handlePythonPluginEvent(const Pothos::Plugin &plugin, const std::string &event)
{
    if (event == "remove" and plugin.getPath() == Pothos::PluginPath("/proxy_helpers/python/pyobject_to_proxy"))
    {
        myPythonProxyEnv.reset();
        myPyObjectToProxyFcn = PyObjectToProxyFcn();
        Pothos::PluginRegistry::remove("/proxy/converters/python/proxy_to_pyproxy");
    }
    if (event == "remove" and plugin.getPath() == Pothos::PluginPath("/proxy_helpers/python/proxy_to_pyobject"))
    {
        myPythonProxyEnv.reset();
        myProxyToPyObjectFcn = ProxyToPyObjectFcn();
        Pothos::PluginRegistry::remove("/proxy/converters/python/pyproxy_to_proxy");
    }
}

Pothos::Proxy PyObjectToProxy(PyObject *obj)
{
    assert(obj != nullptr);
    if (isProxyObject(obj)) return *reinterpret_cast<ProxyObject *>(obj)->proxy;
    PyThreadStateLock lock;
    return myPyObjectToProxyFcn(myPythonProxyEnv, obj);
}

PyObject *ProxyToPyObject(const Pothos::Proxy &proxy)
{
    assert(proxy);
    PyThreadStateLock lock;
    return myProxyToPyObjectFcn(proxy);
}

Pothos::ProxyEnvironment::Sptr getPythonProxyEnv(void)
{
    return myPythonProxyEnv;
}

/***********************************************************************
 * converters to and from pothos proxy type
 **********************************************************************/
static Pothos::Proxy convertPyProxyToProxy(const Pothos::Proxy &proxy)
{
    PyObjectRef ref(ProxyToPyObject(proxy), REF_NEW);
    return *reinterpret_cast<ProxyObject *>(ref.obj)->proxy;
}

static Pothos::Proxy convertProxyToPyProxy(Pothos::ProxyEnvironment::Sptr env, const Pothos::Proxy &proxy)
{
    PyObjectRef ref(makeProxyObject(proxy), REF_NEW);
    PyThreadStateLock lock;
    return myPyObjectToProxyFcn(env, ref.obj);
}

void registerPothosModuleConverters(void)
{
    Pothos::PluginRegistry::addCall("/proxy_helpers/python", &handlePythonPluginEvent);
    Pothos::PluginRegistry::addCall("/proxy/converters/python/proxy_to_pyproxy",
        &convertProxyToPyProxy);
    Pothos::PluginRegistry::add("/proxy/converters/python/pyproxy_to_proxy",
        Pothos::ProxyConvertPair("PothosProxy", &convertPyProxyToProxy));
}

/***********************************************************************
 * module error support
 **********************************************************************/
static PyObject *PothosModuleError;

/***********************************************************************
 * module setup
 **********************************************************************/
#ifdef __GNUC__ //default visibility is hidden, append this attribute to export init module
PyMODINIT_FUNC __attribute__ ((visibility ("default")))
#else
PyMODINIT_FUNC
#endif
#if PY_MAJOR_VERSION >= 3
PyInit_PothosModule(void)
#else
initPothosModule(void)
#endif
{
    initPyObjectUtilityConverters();

    #if PY_MAJOR_VERSION >= 3
    static PyModuleDef PothosModule = {
        PyModuleDef_HEAD_INIT,
        "PothosModule",
        "Pothos python bindings",
        -1,
        nullptr, nullptr, nullptr, nullptr, nullptr
    };
    PyObject *m = PyModule_Create(&PothosModule);
    #else
    PyObject *m = Py_InitModule("PothosModule", nullptr);
    #endif

    if (m != nullptr)
    {
        PothosModuleError = PyErr_NewException((char *)"PothosModule.error", NULL, NULL);
        Py_INCREF(PothosModuleError);
        PyModule_AddObject(m, "error", PothosModuleError);

        registerProxyType(m);
        registerProxyCallType(m);
        registerProxyEnvironmentType(m);
    }

    #if PY_MAJOR_VERSION >= 3
    return m;
    #endif
}
