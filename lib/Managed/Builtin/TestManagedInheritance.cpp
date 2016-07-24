// Copyright (c) 2013-2016 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Plugin.hpp>
#include <Pothos/Proxy.hpp>
#include <Pothos/Managed.hpp>
#include <iostream>

class MyBaseClass0
{
public:
    MyBaseClass0(void)
    {
        return;
    }

    virtual ~MyBaseClass0(void)
    {
        return;
    }

    int negateInt(int val)
    {
        return -val;
    }
};

class MyDerivedClass0 : public MyBaseClass0
{
public:
    MyDerivedClass0(void)
    {
        return;
    }
};

static void test_inheritance0(Pothos::ProxyEnvironment::Sptr env)
{
    Pothos::ManagedClass()
        .registerConstructor<MyBaseClass0>()
        .registerMethod(POTHOS_FCN_TUPLE(MyBaseClass0, negateInt))
        .commit("MyBaseClass0");

    //prove that base class can work
    auto myBase0 = env->findProxy("MyBaseClass0")();
    POTHOS_TEST_EQUAL(-42, myBase0.call<int>("negateInt", 42));

    Pothos::ManagedClass()
        .registerConstructor<MyDerivedClass0>()
        .registerBaseClass<MyDerivedClass0, MyBaseClass0>()
        .commit("MyDerivedClass0");

    //prove that derived class has the base methods
    auto myDerived0 = env->findProxy("MyDerivedClass0")();
    POTHOS_TEST_EQUAL(-42, myDerived0.call<int>("negateInt", 42));

    //runtime registration does not associate the module
    //therefore to be safe, we unregister these classes now
    Pothos::PluginRegistry::remove("/managed/MyBaseClass0");
    Pothos::PluginRegistry::remove("/managed/MyDerivedClass0");
}

POTHOS_TEST_BLOCK("/proxy/managed/tests", test_inheritance)
{
    test_inheritance0(Pothos::ProxyEnvironment::make("managed"));
}
