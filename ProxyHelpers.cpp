// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PythonProxy.hpp"
#include <Pothos/Plugin.hpp>

/***********************************************************************
 * PyObject helpers - used in python bindings
 **********************************************************************/
static Pothos::Proxy convertPyObjectToProxy(Pothos::ProxyEnvironment::Sptr env, PyObject *obj)
{
    return std::dynamic_pointer_cast<PythonProxyEnvironment>(env)->makeHandle(obj, REF_BORROWED);
}

static PyObject *convertProxyToPyObject(const Pothos::Proxy &proxy)
{
    return std::dynamic_pointer_cast<PythonProxyHandle>(proxy.getHandle())->ref.newRef();
}

pothos_static_block(pothosRegisterPyObjectHelpers)
{
    Pothos::PluginRegistry::add("/proxy_helpers/python/pyobject_to_proxy",
        PyObjectToProxyFcn(&convertPyObjectToProxy));
    Pothos::PluginRegistry::add("/proxy_helpers/python/proxy_to_pyobject",
        ProxyToPyObjectFcn(&convertProxyToPyObject));
}
