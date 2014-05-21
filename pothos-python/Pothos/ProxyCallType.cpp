// Copyright (c) 2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PothosModule.hpp"
#include <cassert>

static PyTypeObject ProxyCallType = {
    PyObject_HEAD_INIT(NULL)
};

static void ProxyCall_dealloc(ProxyCallObject *self)
{
    delete self->proxy;
    delete self->name;
    Py_TYPE(self)->tp_free((PyObject*)self);
}

static int ProxyCall_init(ProxyCallObject *self, PyObject *args, PyObject *)
{
    self->proxy = new PyObjectRef(PyTuple_GetItem(args, 0), REF_BORROWED);
    self->name = new PyObjectRef(PyTuple_GetItem(args, 1), REF_BORROWED);
    return 0;
}

PyObject* ProxyCall_call(ProxyCallObject *self, PyObject *args, PyObject *)
{
    try
    {
        //extract string name
        const auto name = PyObjectToProxy(self->name->obj).convert<std::string>();

        //create call args
        Pothos::ProxyVector proxyArgs;
        for (int i = 0; i < PyTuple_Size(args); i++)
        {
            proxyArgs.push_back(PyObjectToProxyInspect(PyTuple_GetItem(args, i)));
        }

        //make proxy call
        auto handle = ((ProxyObject *)self->proxy->obj)->proxy->getHandle();
        Pothos::Proxy proxy;
        {
            PyThreadStateLock lock; //proxy call could be potentially blocking
            proxy = handle->call(name, proxyArgs.data(), proxyArgs.size());
        }

        //convert the result into a pyobject
        return ProxyToPyObject(proxyEnvTranslate(proxy, getPythonProxyEnv()));
    }
    catch (const Pothos::Exception &ex)
    {
        PyErr_SetString(PyExc_RuntimeError, ex.displayText().c_str());
        return nullptr;
    }
}

PyObject *makeProxyCallObject(PyObject *args)
{
    return PyObject_CallObject((PyObject *)&ProxyCallType, args);
}

void registerProxyCallType(PyObject *m)
{
    ProxyCallType.tp_new = PyType_GenericNew;
    ProxyCallType.tp_name = "PothosProxyCall";
    ProxyCallType.tp_basicsize = sizeof(ProxyCallObject);
    ProxyCallType.tp_dealloc = (destructor)ProxyCall_dealloc;
    ProxyCallType.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
    ProxyCallType.tp_doc = "Pothos Proxy Call binding";
    ProxyCallType.tp_init = (initproc)ProxyCall_init;
    ProxyCallType.tp_call = (ternaryfunc)ProxyCall_call;

    if (PyType_Ready(&ProxyCallType) < 0) return;

    Py_INCREF(&ProxyCallType);
    PyModule_AddObject(m, "ProxyCall", (PyObject *)&ProxyCallType);
}
