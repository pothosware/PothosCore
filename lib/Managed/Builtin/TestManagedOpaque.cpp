// Copyright (c) 2014-2017 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Plugin.hpp>
#include <Pothos/Proxy.hpp>
#include <Pothos/Managed.hpp>
#include <iostream>

struct SuperOpaque
{
    SuperOpaque(const Pothos::Object *args, const size_t numArgs)
    {
        if (numArgs > 0) _value = args[0].operator std::string();
    }

    Pothos::Object foo(const Pothos::Object *args, const size_t numArgs)
    {
        if (numArgs > 0) _value = args[0].operator std::string();
        return Pothos::Object(_value);
    }

    static Pothos::Object bar(const Pothos::Object *args, const size_t numArgs)
    {
        if (numArgs > 0) return Pothos::Object(args[0].operator std::string());
        return Pothos::Object("");
    }

    std::string _value;
};

POTHOS_TEST_BLOCK("/proxy/managed/tests", test_opaque_calls)
{
    Pothos::ManagedClass()
        .registerOpaqueConstructor<SuperOpaque>()
        .registerOpaqueMethod(POTHOS_FCN_TUPLE(SuperOpaque, foo))
        .registerOpaqueStaticMethod(POTHOS_FCN_TUPLE(SuperOpaque, bar))
        .commit("SuperOpaque");

    {
        auto env = Pothos::ProxyEnvironment::make("managed");
        auto SuperOpaqueProxy = env->findProxy("SuperOpaque");

        //we should be setting and getting an empty string
        auto sop0 = SuperOpaqueProxy();
        POTHOS_TEST_EQUAL(sop0.call<std::string>("foo"), std::string(""));

        //change the value
        POTHOS_TEST_EQUAL(sop0.call<std::string>("foo", "hello"), std::string("hello"));

        //initial value
        auto sop1 = SuperOpaqueProxy("world");
        POTHOS_TEST_EQUAL(sop1.call<std::string>("foo"), std::string("world"));

        //static test
        POTHOS_TEST_EQUAL(SuperOpaqueProxy.call<std::string>("bar", "test"), std::string("test"));
    }

    //runtime registration does not associate the module
    //therefore to be safe, we unregister these classes now
    Pothos::PluginRegistry::remove("/managed/SuperOpaque");
}
