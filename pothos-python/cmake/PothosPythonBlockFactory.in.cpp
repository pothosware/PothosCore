// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <Pothos/Proxy.hpp>

static Pothos::Object @class_name@Factory(const Pothos::Object *args, const size_t numArgs)
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
    auto mod = env->findProxy("@package_name@");

    //call into the factory
    auto block = mod.getHandle()->call("@class_name@", proxyArgs.data(), proxyArgs.size());
    return Pothos::Object(block);
}

static Pothos::BlockRegistry register@class_name@("@block_path@", &@class_name@Factory);
