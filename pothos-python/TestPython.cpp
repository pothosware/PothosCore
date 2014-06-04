// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Proxy.hpp>
#include <iostream>
#include <complex>
#include <cstdlib>

POTHOS_TEST_BLOCK("/proxy/python/tests", test_basic_types)
{
    auto env = Pothos::ProxyEnvironment::make("python");

    auto noneProxy = env->convertObjectToProxy(Pothos::Object());
    auto nullObject = env->convertProxyToObject(noneProxy);
    POTHOS_TEST_TRUE(not nullObject);

    POTHOS_TEST_EQUAL(env->makeProxy(true).convert<bool>(), true);
    POTHOS_TEST_EQUAL(env->makeProxy(false).convert<bool>(), false);

    const int intVal = int(std::rand()-RAND_MAX/2);
    POTHOS_TEST_EQUAL(env->makeProxy(intVal).convert<int>(), intVal);

    const long long longVal = (long long)(std::rand()-RAND_MAX/2);
    POTHOS_TEST_EQUAL(env->makeProxy(longVal).convert<long long>(), longVal);

    const double floatVal = double(std::rand()-RAND_MAX/2);
    POTHOS_TEST_EQUAL(env->makeProxy(floatVal).convert<double>(), floatVal);

    const std::complex<double> complexVal(1.0*(std::rand()-RAND_MAX/2), 1.0*(std::rand()-RAND_MAX/2));
    POTHOS_TEST_EQUAL(env->makeProxy(complexVal).convert<std::complex<double>>(), complexVal);

    const std::string strVal = "Hello World!";
    POTHOS_TEST_EQUAL(env->makeProxy(strVal).convert<std::string>(), strVal);
}

POTHOS_TEST_BLOCK("/proxy/python/tests", test_compare_to)
{
    auto env = Pothos::ProxyEnvironment::make("python");

    auto int0 = env->makeProxy(0);
    auto int1 = env->makeProxy(1);
    auto int2 = env->makeProxy(2);

    //test compare less, greater...
    POTHOS_TEST_TRUE(int0 < int1);
    POTHOS_TEST_TRUE(int0 < int2);
    POTHOS_TEST_TRUE(int1 < int2);
    POTHOS_TEST_TRUE(int0 < int2);

    POTHOS_TEST_TRUE(int1 > int0);
    POTHOS_TEST_TRUE(int2 > int0);
    POTHOS_TEST_TRUE(int2 > int1);
    POTHOS_TEST_TRUE(int2 > int0);

    //test equality with same value, different proxy
    auto int2Again = env->makeProxy(2);
    POTHOS_TEST_EQUAL(int2.compareTo(int2), 0);
    POTHOS_TEST_EQUAL(int2.compareTo(int2Again), 0);
}

POTHOS_TEST_BLOCK("/proxy/python/tests", test_containers)
{
    auto env = Pothos::ProxyEnvironment::make("python");

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

POTHOS_TEST_BLOCK("/proxy/python/tests", test_call_module)
{
    auto env = Pothos::ProxyEnvironment::make("python");

    auto re = env->findProxy("re");
    auto m = re.callProxy("search", "(?<=abc)def", "abcdef");
    auto group = m.call<std::string>("group", 0);
    POTHOS_TEST_EQUAL(group, "def");


    Pothos::ProxyMap myDict;
    myDict[env->makeProxy("hi")] = env->makeProxy("bye");
    myDict[env->makeProxy(1)] = env->makeProxy(2);
}
