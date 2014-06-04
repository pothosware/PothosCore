// Copyright (c) 2013-2013 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Proxy.hpp>
#include <iostream>
#include <vector>
#include <cstdlib>

POTHOS_TEST_BLOCK("/proxy/java/tests", test_basic_types)
{
    auto env = Pothos::ProxyEnvironment::make("java");

    auto noneProxy = env->convertObjectToProxy(Pothos::Object());
    auto nullObject = env->convertProxyToObject(noneProxy);
    POTHOS_TEST_TRUE(not nullObject);

    POTHOS_TEST_EQUAL(env->makeProxy(true).convert<bool>(), true);
    POTHOS_TEST_EQUAL(env->makeProxy(false).convert<bool>(), false);

    const char charVal = char((std::rand()-RAND_MAX/2)/(RAND_MAX >> 8));
    POTHOS_TEST_EQUAL(env->makeProxy(charVal).convert<char>(), charVal);

    const short shortVal = short((std::rand()-RAND_MAX/2));
    POTHOS_TEST_EQUAL(env->makeProxy(shortVal).convert<short>(), shortVal);

    const int intVal = int(std::rand()-RAND_MAX/2);
    POTHOS_TEST_EQUAL(env->makeProxy(intVal).convert<int>(), intVal);

    const long longVal = long(std::rand()-RAND_MAX/2);
    POTHOS_TEST_EQUAL(env->makeProxy(longVal).convert<long>(), longVal);

    const long long llongVal = (long long)(std::rand()-RAND_MAX/2);
    POTHOS_TEST_EQUAL(env->makeProxy(llongVal).convert<long long>(), llongVal);

    const float floatVal = float(std::rand()-RAND_MAX/2);
    POTHOS_TEST_EQUAL(env->makeProxy(floatVal).convert<float>(), floatVal);

    const double doubleVal = double(std::rand()-RAND_MAX/2);
    POTHOS_TEST_EQUAL(env->makeProxy(doubleVal).convert<double>(), doubleVal);

    const std::string strVal = "Hello World!";
    POTHOS_TEST_EQUAL(env->makeProxy(strVal).convert<std::string>(), strVal);
}

POTHOS_TEST_BLOCK("/proxy/java/tests", test_compare_to)
{
    auto env = Pothos::ProxyEnvironment::make("java");

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

template <typename T>
void testPrimArray(Pothos::ProxyEnvironment::Sptr env, const long div)
{
    //create test vector
    std::vector<T> testVec(17);
    for (size_t i = 0; i < testVec.size(); i++) testVec[i] = T((std::rand()-RAND_MAX/2)/div);

    //convert into proxy
    Pothos::Proxy arrayProxy = env->makeProxy(testVec);

    //check result
    auto resultVec = arrayProxy.convert<std::vector<T>>();
    POTHOS_TEST_EQUALV(testVec, resultVec);
}

POTHOS_TEST_BLOCK("/proxy/java/tests", test_prim_array)
{
    auto env = Pothos::ProxyEnvironment::make("java");
    testPrimArray<char>(env, (RAND_MAX >> 8));
    testPrimArray<short>(env, 1);
    testPrimArray<int>(env, 1);
    testPrimArray<long>(env, 1);
    testPrimArray<long long>(env, 1);
    testPrimArray<float>(env, 1);
    testPrimArray<double>(env, 1);
}

POTHOS_TEST_BLOCK("/proxy/java/tests", test_containers)
{
    auto env = Pothos::ProxyEnvironment::make("java");

    //create test vector
    Pothos::ProxyVector testVec;
    testVec.push_back(env->makeProxy(42));
    testVec.push_back(env->makeProxy("test it"));
    testVec.push_back(env->makeProxy(-21.0));

    //convert into proxy
    auto arrayProxy = env->makeProxy(testVec);
    auto vecObj = arrayProxy.convert<Pothos::ProxyVector>();

    //check result
    POTHOS_TEST_EQUAL(testVec.size(), vecObj.size());
    POTHOS_TEST_EQUAL(testVec[0].convert<int>(), vecObj[0].convert<int>());
    POTHOS_TEST_EQUAL(testVec[1].convert<std::string>(), vecObj[1].convert<std::string>());
    POTHOS_TEST_EQUAL(testVec[2].convert<double>(), vecObj[2].convert<double>());

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

POTHOS_TEST_BLOCK("/proxy/java/tests", test_reflect)
{
    auto env = Pothos::ProxyEnvironment::make("java");
    const std::string str = "this is a test";
    //use the method named "class" to get reflective info:

    //do we get the names expected from the reflect class?
    auto myString = env->makeProxy(str);
    auto strClass = myString.callProxy("class");
    POTHOS_TEST_EQUAL(strClass.call<std::string>("getName"), "java.lang.String");

    auto strBytes = myString.callProxy("getBytes");
    auto bytesClass = strBytes.callProxy("class");
    POTHOS_TEST_EQUAL(bytesClass.call<std::string>("getName"), "[B");

    //check the conversion as well
    auto bytesVec = myString.call<std::vector<char>>("getBytes");
    POTHOS_TEST_EQUALV(bytesVec, str);
}

POTHOS_TEST_BLOCK("/proxy/java/tests", test_primitive)
{
    auto env = Pothos::ProxyEnvironment::make("java");

    auto myBoolean = env->findProxy("java.lang.Boolean").callProxy("new", true);
    POTHOS_TEST_EQUAL(myBoolean.convert<bool>(), true);

    auto myCharacter = env->findProxy("java.lang.Character").callProxy("new", 21);
    POTHOS_TEST_EQUAL(myCharacter.convert<char>(), 21);

    auto myByte = env->findProxy("java.lang.Byte").callProxy("new", -12);
    POTHOS_TEST_EQUAL(myByte.convert<signed char>(), -12);

    auto myShort = env->findProxy("java.lang.Short").callProxy("new", 12345);
    POTHOS_TEST_EQUAL(myShort.convert<short>(), 12345);

    auto myInteger = env->findProxy("java.lang.Integer").callProxy("new", 42);
    POTHOS_TEST_EQUAL(myInteger.convert<int>(), 42);

    auto myLong = env->findProxy("java.lang.Long").callProxy("new", 1ll << 34);
    POTHOS_TEST_EQUAL(myLong.convert<long long>(), 1ll << 34);

    auto myFloat = env->findProxy("java.lang.Float").callProxy("new", -30.0);
    POTHOS_TEST_EQUAL(myFloat.convert<float>(), -30.0);

    auto myDouble = env->findProxy("java.lang.Double").callProxy("new", -20.0);
    POTHOS_TEST_EQUAL(myDouble.convert<double>(), -20.0);
}

POTHOS_TEST_BLOCK("/proxy/java/tests", test_fields)
{
    auto env = Pothos::ProxyEnvironment::make("java");

    auto myInteger = env->findProxy("java.lang.Integer").callProxy("new", 42);
    std::cout << myInteger.call<int>("get:MAX_VALUE") << std::endl;
    //TODO try to set and get a non static field when we find an example object to try it on
}
