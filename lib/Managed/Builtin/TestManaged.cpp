// Copyright (c) 2013-2016 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Plugin.hpp>
#include <Pothos/Proxy.hpp>
#include <Pothos/Remote.hpp>
#include <Pothos/Managed.hpp>
#include <iostream>
#include <complex>

POTHOS_TEST_BLOCK("/proxy/managed/tests", test_containers)
{
    auto env = Pothos::ProxyEnvironment::make("managed");

    //make test vector
    Pothos::ProxyVector testVec;
    testVec.push_back(env->makeProxy("mytest"));
    testVec.push_back(env->makeProxy(42));
    testVec.push_back(env->makeProxy(std::complex<double>(1, 2)));

    //convert to proxy and back
    auto proxyVec = env->makeProxy(testVec);
    auto resultVec = proxyVec.convert<Pothos::ProxyVector>();

    //check for equality
    POTHOS_TEST_EQUAL(testVec.size(), resultVec.size());
    POTHOS_TEST_EQUAL(testVec[0].convert<std::string>(), resultVec[0].convert<std::string>());
    POTHOS_TEST_EQUAL(testVec[1].convert<int>(), resultVec[1].convert<int>());
    POTHOS_TEST_EQUAL(testVec[2].convert<std::complex<double>>(), resultVec[2].convert<std::complex<double>>());

    //make test set
    Pothos::ProxySet testSet;
    testSet.insert(env->makeProxy("hi"));
    testSet.insert(env->makeProxy(1));

    //convert to proxy and back
    auto proxySet = env->makeProxy(testSet);
    auto resultSet = proxySet.convert<Pothos::ProxySet>();

    //check result
    auto findHiSet = resultSet.find(env->makeProxy("hi"));
    POTHOS_TEST_TRUE(findHiSet != resultSet.end());
    auto find1Set = resultSet.find(env->makeProxy(1));
    POTHOS_TEST_TRUE(find1Set != resultSet.end());

    //make test dictionary
    Pothos::ProxyMap testDict;
    testDict[env->makeProxy("hi")] = env->makeProxy("bye");
    testDict[env->makeProxy(1)] = env->makeProxy(2);

    //convert to proxy and back
    auto proxyDict = env->makeProxy(testDict);
    auto resultDict = proxyDict.convert<Pothos::ProxyMap>();

    //check result
    auto findHi = resultDict.find(env->makeProxy("hi"));
    POTHOS_TEST_TRUE(findHi != resultDict.end());
    POTHOS_TEST_EQUAL(findHi->second.convert<std::string>(), "bye");
    auto find1 = resultDict.find(env->makeProxy(1));
    POTHOS_TEST_TRUE(find1 != resultDict.end());
    POTHOS_TEST_EQUAL(find1->second.convert<int>(), 2);
}

struct TestAccessFunctions
{
    TestAccessFunctions(void):
        _value(0)
    {
        return;
    }

    static long addNums(const int num0, const int num1)
    {
        return num0 + num1;
    }

    static Pothos::Object addNums2(const Pothos::Object *args, const size_t numArgs)
    {
        if (numArgs > 1) return Pothos::Object(args[0].convert<int>() + args[1].convert<int>());
        return Pothos::Object("");
    }

    void setValue(const int value)
    {
        _value = value;
    }

    int getValue(void) const
    {
        return _value;
    }

    Pothos::Object setAndGetValue(const Pothos::Object *args, const size_t numArgs)
    {
        if (numArgs > 0) this->setValue(args[0].convert<int>());
        return Pothos::Object(this->getValue());
    }

    int _value;
};

POTHOS_TEST_BLOCK("/proxy/managed/tests", test_function_accessors)
{
    Pothos::ManagedClass()
        .registerConstructor<TestAccessFunctions>()
        .registerStaticMethod(POTHOS_FCN_TUPLE(TestAccessFunctions, addNums))
        .registerOpaqueStaticMethod(POTHOS_FCN_TUPLE(TestAccessFunctions, addNums2))
        .registerMethod(POTHOS_FCN_TUPLE(TestAccessFunctions, setValue))
        .registerMethod(POTHOS_FCN_TUPLE(TestAccessFunctions, getValue))
        .registerOpaqueMethod(POTHOS_FCN_TUPLE(TestAccessFunctions, setAndGetValue))
        .commit("TestAccessFunctions");

    {
        auto env = Pothos::ProxyEnvironment::make("managed");
        auto TestAccessFunctionsCls = env->findProxy("TestAccessFunctions");

        //get a function for a static method
        auto addNumsFcn = TestAccessFunctionsCls.get("addNums");
        POTHOS_TEST_EQUAL(addNumsFcn(1, 2).convert<long>(), 3);

        //get a function for an opaque static method
        auto addNums2Fcn = TestAccessFunctionsCls.get("addNums2");
        POTHOS_TEST_EQUAL(addNums2Fcn(3, 4).convert<long>(), 7);

        //get functions for a class method
        auto inst0 = TestAccessFunctionsCls();
        auto setValueInst0 = inst0.get("setValue");
        auto getValueInst0 = inst0.get("getValue");
        setValueInst0(1234);
        POTHOS_TEST_EQUAL(getValueInst0().convert<int>(), 1234);

        //get opaque function for a class method
        auto setAndGetValueInst0 = inst0.get("setAndGetValue");
        POTHOS_TEST_EQUAL(setAndGetValueInst0(5678).convert<int>(), 5678);
    }

    //runtime registration does not associate the module
    //therefore to be safe, we unregister these classes now
    Pothos::PluginRegistry::remove("/managed/TestAccessFunctions");
}
