// Copyright (c) 2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <Pothos/Proxy.hpp>

inline Pothos::Object pothosPythonBlockFactory(const std::string &pythonModulePath, const Pothos::Object *args, const size_t numArgs)
{
    //create python environment
    auto env = Pothos::ProxyEnvironment::make("python");
    env->findProxy("Pothos"); //required Pothos module import -- registers important converters

    //convert arguments into proxy environment
    std::vector<Pothos::Proxy> proxyArgs(numArgs);
    for (size_t i = 0; i < numArgs; i++)
    {
        proxyArgs[i] = env->convertObjectToProxy(args[i]);
    }

    //locate the module
    auto lastDot = pythonModulePath.find_last_of(".");
    auto modPath = pythonModulePath.substr(0, lastDot);
    auto factoryName = pythonModulePath.substr(lastDot+1);
    auto mod = env->findProxy(modPath);

    //call into the factory
    auto block = mod.getHandle()->call(factoryName, proxyArgs.data(), proxyArgs.size());
    return Pothos::Object(block);
}
