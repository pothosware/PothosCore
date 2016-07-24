// Copyright (c) 2014-2016 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Plugin.hpp>
#include <Pothos/Proxy.hpp>
#include <Pothos/Managed.hpp>
#include <iostream>

struct SuperWildcard
{
    SuperWildcard(const Pothos::Object *args, const size_t numArgs)
    {
        if (numArgs > 0) _value = args[0].convert<std::string>();
    }

    Pothos::Object foo(const std::string &name, const Pothos::Object *args, const size_t numArgs)
    {
        if (name == "foo")
        {
            if (numArgs > 0) _value = args[0].convert<std::string>();
            return Pothos::Object(_value);
        }
        return Pothos::Object();
    }

    static Pothos::Object bar(const std::string &name, const Pothos::Object *args, const size_t numArgs)
    {
        if (name == "bar")
        {
            if (numArgs > 0) return Pothos::Object(args[0].convert<std::string>());
            return Pothos::Object("");
        }
        return Pothos::Object();
    }

    std::string _value;
};

POTHOS_TEST_BLOCK("/proxy/managed/tests", test_wildcard_calls)
{
    Pothos::ManagedClass()
        .registerOpaqueConstructor<SuperWildcard>()
        .registerWildcardMethod(&SuperWildcard::foo)
        .registerWildcardStaticMethod(&SuperWildcard::bar)
        .commit("SuperWildcard");

    {
        auto env = Pothos::ProxyEnvironment::make("managed");
        auto SuperWildcardProxy = env->findProxy("SuperWildcard");

        //we should be setting and getting an empty string
        auto sop0 = SuperWildcardProxy();
        POTHOS_TEST_EQUAL(sop0.call<std::string>("foo"), std::string(""));

        //change the value
        POTHOS_TEST_EQUAL(sop0.call<std::string>("foo", "hello"), std::string("hello"));

        //initial value
        auto sop1 = SuperWildcardProxy("world");
        POTHOS_TEST_EQUAL(sop1.call<std::string>("foo"), std::string("world"));

        //static test
        POTHOS_TEST_EQUAL(SuperWildcardProxy.call<std::string>("bar", "test"), std::string("test"));
    }

    //runtime registration does not associate the module
    //therefore to be safe, we unregister these classes now
    Pothos::PluginRegistry::remove("/managed/SuperWildcard");
}
