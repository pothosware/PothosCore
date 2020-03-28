// Copyright (c) 2013-2014 Josh Blum
//                    2020 Nicholas Corgan
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
    Pothos::ProxyVector resultVec = proxyVec;

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
    Pothos::ProxySet resultSet = proxySet;

    //check result
    auto findHiSet = resultSet.find(env->makeProxy("hi"));
    POTHOS_TEST_NOT_EQUAL(findHiSet, resultSet.end());
    auto find1Set = resultSet.find(env->makeProxy(1));
    POTHOS_TEST_NOT_EQUAL(find1Set, resultSet.end());

    //make test dictionary
    Pothos::ProxyMap testDict;
    testDict[env->makeProxy("hi")] = env->makeProxy("bye");
    testDict[env->makeProxy(1)] = env->makeProxy(2);

    //convert to proxy and back
    auto proxyDict = env->makeProxy(testDict);
    Pothos::ProxyMap resultDict = proxyDict;

    //check result
    auto findHi = resultDict.find(env->makeProxy("hi"));
    POTHOS_TEST_NOT_EQUAL(findHi, resultDict.end());
    POTHOS_TEST_EQUAL(findHi->second.convert<std::string>(), "bye");
    auto find1 = resultDict.find(env->makeProxy(1));
    POTHOS_TEST_NOT_EQUAL(find1, resultDict.end());
    POTHOS_TEST_EQUAL(find1->second.convert<int>(), 2);
}
