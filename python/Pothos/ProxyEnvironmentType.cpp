// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PothosModule.hpp"
#include <cassert>

static PyTypeObject ProxyEnvironmentType = {
    PyObject_HEAD_INIT(NULL)
};

static void ProxyEnvironment_dealloc(ProxyEnvironmentObject *self)
{
    delete self->env;
    Py_TYPE(self)->tp_free((PyObject*)self);
}

static int ProxyEnvironment_init(ProxyEnvironmentObject *self, PyObject *args, PyObject *kwds)
{
    //convert args, should not throw
    Pothos::ProxyVector proxyArgs;
    Pothos::ProxyMap proxyKwargs;
    if (args != nullptr) proxyArgs = PyObjectToProxy(args).convert<Pothos::ProxyVector>();
    if (kwds != nullptr) proxyKwargs = PyObjectToProxy(kwds).convert<Pothos::ProxyMap>();

    //allocate environment sptr
    self->env = new Pothos::ProxyEnvironment::Sptr();

    //no args, thats ok, its a null env
    if (proxyArgs.empty()) return 0;

    //check the input
    if (proxyArgs.size() != 1)
    {
        PyErr_SetString(PyExc_RuntimeError, "expects one arg for proxy name");
        return -1;
    }
    try
    {
        Pothos::ProxyEnvironmentArgs envArgs;
        for (const auto &pair : proxyKwargs)
        {
            envArgs[pair.first.convert<std::string>()] = pair.second.convert<std::string>();
        }
        const auto name = proxyArgs[0].convert<std::string>();
        *(self->env) = Pothos::ProxyEnvironment::make(name, envArgs);
    }
    catch (const Pothos::Exception &ex)
    {
        PyErr_SetString(PyExc_RuntimeError, ex.displayText().c_str());
        return -1;
    }

    return 0;
}

static PyObject *ProxyEnvironment_findProxy(ProxyEnvironmentObject *self, PyObject *args)
{
    //convert args, should not throw
    Pothos::ProxyVector proxyArgs;
    if (args != nullptr) proxyArgs = PyObjectToProxy(args).convert<Pothos::ProxyVector>();

    //check the input
    if (proxyArgs.size() != 1)
    {
        PyErr_SetString(PyExc_RuntimeError, "expects one arg for proxy name");
        return nullptr;
    }
    try
    {
        const auto name = proxyArgs[0].convert<std::string>();
        auto proxy = (*self->env)->findProxy(name);
        return makeProxyObject(proxy);
    }
    catch (const Pothos::Exception &ex)
    {
        PyErr_SetString(PyExc_RuntimeError, ex.displayText().c_str());
        return nullptr;
    }
}

static PyObject *ProxyEnvironment_getName(ProxyEnvironmentObject *self)
{
    const auto name = (*self->env)->getName();
    auto proxy = getPythonProxyEnv()->makeProxy(name);
    return ProxyToPyObject(proxy);
}

static PyObject *ProxyEnvironment_convertObjectToProxy(ProxyEnvironmentObject *self, PyObject *args)
{
    //check the input
    if (not args or PyTuple_Size(args) != 1)
    {
        PyErr_SetString(PyExc_RuntimeError, "expects one arg for input object");
        return nullptr;
    }
    auto arg0 = PyTuple_GetItem(args, 0);
    if (isProxyObject(arg0))
    {
        PyErr_SetString(PyExc_RuntimeError, "input should not be of type ProxyObject");
        return nullptr;
    }

    //convert into the environment in self
    try
    {
        auto proxy = PyObjectToProxy(arg0);
        return makeProxyObject(proxyEnvTranslate(proxy, *self->env));
    }
    catch (const Pothos::Exception &ex)
    {
        PyErr_SetString(PyExc_RuntimeError, ex.displayText().c_str());
        return nullptr;
    }
}

static PyObject *ProxyEnvironment_convertProxyToObject(ProxyEnvironmentObject *, PyObject *args)
{
    //check the input
    if (not args or PyTuple_Size(args) != 1)
    {
        PyErr_SetString(PyExc_RuntimeError, "expects one arg for input proxy");
        return nullptr;
    }
    auto arg0 = PyTuple_GetItem(args, 0);
    if (not isProxyObject(arg0))
    {
        PyErr_SetString(PyExc_RuntimeError, "input is not of type ProxyObject");
        return nullptr;
    }

    //convert proxy into a proxy holding a python object
    try
    {
        auto proxy = *reinterpret_cast<ProxyObject *>(arg0)->proxy;
        return ProxyToPyObject(proxyEnvTranslate(proxy, getPythonProxyEnv()));
    }
    catch (const Pothos::Exception &ex)
    {
        PyErr_SetString(PyExc_RuntimeError, ex.displayText().c_str());
        return nullptr;
    }
}

static PyMethodDef ProxyEnvironment_methods[] = {
    {"findProxy", (PyCFunction)ProxyEnvironment_findProxy, METH_VARARGS, "Pothos::ProxyEnvironment::findProxy(name)"},
    {"getName", (PyCFunction)ProxyEnvironment_getName, METH_NOARGS, "Pothos::ProxyEnvironment::getName()"},
    {"convertObjectToProxy", (PyCFunction)ProxyEnvironment_convertObjectToProxy, METH_VARARGS, "Pothos::ProxyEnvironment::convertObjectToProxy(obj)"},
    {"convertProxyToObject", (PyCFunction)ProxyEnvironment_convertProxyToObject, METH_VARARGS, "Pothos::ProxyEnvironment::convertProxyToObject(proxy)"},
    {nullptr}  /* Sentinel */
};

PyObject* ProxyEnvironment_Compare(PyObject *o1, PyObject *o2, int opid)
{
    if (not isProxyEnvironmentObject(o1) or not isProxyEnvironmentObject(o2))
    {
        PyErr_SetString(PyExc_RuntimeError, "ProxyEnvironment must compare to another ProxyEnvironment");
        return nullptr;
    }

    auto s1 = *reinterpret_cast<ProxyEnvironmentObject*>(o1)->env;
    auto s2 = *reinterpret_cast<ProxyEnvironmentObject*>(o2)->env;
    const int cmp = (s1 < s2)? -1 : ((s1 > s2)? +1 : 0);

    return richCompareFromSimple(cmp, opid);
}

PyObject *makeProxyEnvironmentObject(const Pothos::ProxyEnvironment::Sptr &env)
{
    PyObject *o = PyObject_CallObject((PyObject *)&ProxyEnvironmentType, nullptr);
    auto proxyObject = reinterpret_cast<ProxyEnvironmentObject *>(o);
    *(proxyObject->env) = env;
    return o;
}

bool isProxyEnvironmentObject(PyObject *obj)
{
    if (obj == nullptr) return false;
    return Py_TYPE(obj) == &ProxyEnvironmentType;
}

void registerProxyEnvironmentType(PyObject *m)
{
    ProxyEnvironmentType.tp_new = PyType_GenericNew;
    ProxyEnvironmentType.tp_name = "PothosProxyEnvironment";
    ProxyEnvironmentType.tp_basicsize = sizeof(ProxyEnvironmentObject);
    ProxyEnvironmentType.tp_dealloc = (destructor)ProxyEnvironment_dealloc;
    ProxyEnvironmentType.tp_richcompare = (richcmpfunc)ProxyEnvironment_Compare;
    ProxyEnvironmentType.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
    ProxyEnvironmentType.tp_doc = "Pothos ProxyEnvironment binding";
    ProxyEnvironmentType.tp_methods = ProxyEnvironment_methods;
    ProxyEnvironmentType.tp_init = (initproc)ProxyEnvironment_init;

    if (PyType_Ready(&ProxyEnvironmentType) < 0) return;

    Py_INCREF(&ProxyEnvironmentType);
    PyModule_AddObject(m, "ProxyEnvironment", (PyObject *)&ProxyEnvironmentType);
}
