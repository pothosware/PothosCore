// Copyright (c) 2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <Pothos/Proxy.hpp>

inline Pothos::Object pothosPythonBlockFactory(const std::string &packageName, const std::string &className, const Pothos::Object *args, const size_t numArgs)
{
    //create python environment
    auto env = Pothos::ProxyEnvironment::make("python");

    //convert arguments into proxy environment
    std::vector<Pothos::Proxy> proxyArgs(numArgs);
    for (size_t i = 0; i < numArgs; i++)
    {
        proxyArgs[i] = env->convertObjectToProxy(args[i]);
    }

    //locate the module
    auto mod = env->findProxy(packageName);

    //call into the factory
    auto block = mod.getHandle()->call(className, proxyArgs.data(), proxyArgs.size());
    return Pothos::Object(block);
}
