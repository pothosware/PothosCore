// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Plugin.hpp>
#include <Pothos/Proxy.hpp>
#include <Pothos/Remote.hpp>
#include <Pothos/Managed.hpp>
#include <Poco/Pipe.h>
#include <Poco/PipeStream.h>
#include <Poco/Thread.h>
#include <Poco/Runnable.h>
#include <Poco/URI.h>
#include <iostream>
#include <cstdlib>
#include <complex>
#include <algorithm>
#include <cstdlib> //atoi

class SuperBar
{
public:

    SuperBar(void)
    {
        _bar = 0;
    }

    SuperBar(int bar)
    {
        _bar = bar;
    }

    void setBar(int bar)
    {
        _bar = bar;
    }

    int getBar(void)
    {
        return _bar;
    }

    static int what(void)
    {
        return 42;
    }

    int _bar;
};

class SuperFoo
{
public:

    static SuperBar make(int bar)
    {
        return SuperBar(bar);
    }

    static SuperBar *makeNew(int bar)
    {
        return new SuperBar(bar);
    }

    static std::shared_ptr<SuperBar> makeShared(int bar)
    {
        return std::shared_ptr<SuperBar>(makeNew(bar));
    }
};

static void test_simple_runner(Pothos::ProxyEnvironment::Sptr env)
{

    Pothos::ManagedClass()
        .registerConstructor<SuperBar>()
        .registerConstructor<SuperBar, int>()
        .registerMethod(POTHOS_FCN_TUPLE(SuperBar, setBar))
        .registerMethod(POTHOS_FCN_TUPLE(SuperBar, getBar))
        .registerStaticMethod(POTHOS_FCN_TUPLE(SuperBar, what))
        .registerField(POTHOS_FCN_TUPLE(SuperBar, _bar))
        .commit("SuperBar");

    //grab the proxy class
    auto superBarProxy = env->findProxy("SuperBar");

    //test a static method
    POTHOS_TEST_EQUAL(superBarProxy.call<int>("what"), 42);

    //make an instance and test
    auto superBarInstance0 = superBarProxy.callProxy("new");
    superBarInstance0.callVoid("setBar", 123);
    POTHOS_TEST_EQUAL(superBarInstance0.call<int>("getBar"), 123);

    //test field access
    superBarInstance0.callVoid("set:_bar", 321);
    POTHOS_TEST_EQUAL(superBarInstance0.call<int>("get:_bar"), 321);

    //make an instance and test
    auto superBarInstance1 = superBarProxy.callProxy("new", 21);
    POTHOS_TEST_EQUAL(superBarInstance1.call<int>("getBar"), 21);

    Pothos::ManagedClass()
        .registerClass<SuperFoo>()
        .registerStaticMethod(POTHOS_FCN_TUPLE(SuperFoo, make))
        .registerStaticMethod(POTHOS_FCN_TUPLE(SuperFoo, makeNew))
        .registerStaticMethod(POTHOS_FCN_TUPLE(SuperFoo, makeShared))
        .commit("SuperFoo");

    //grab the proxy class
    auto superFooProxy = env->findProxy("SuperFoo");

    //test it making a super bar and test
    auto superBarInstance2 = superFooProxy.callProxy("make", 4567);
    POTHOS_TEST_EQUAL(superBarInstance2.call<int>("getBar"), 4567);
    auto superBarInstance3 = superFooProxy.callProxy("makeNew", -543);
    POTHOS_TEST_EQUAL(superBarInstance3.call<int>("getBar"), -543);
    superBarInstance3.callVoid("delete");
    auto superBarInstance4 = superFooProxy.callProxy("makeShared", 987);
    POTHOS_TEST_EQUAL(superBarInstance4.call<int>("getBar"), 987);

    //runtime registration does not associate the module
    //therefore to be safe, we unregister these classes now
    Pothos::PluginRegistry::remove("/managed/SuperBar");
    Pothos::PluginRegistry::remove("/managed/SuperFoo");
}

POTHOS_TEST_BLOCK("/proxy/managed/tests", test_simple)
{
    test_simple_runner(Pothos::ProxyEnvironment::make("managed"));
}

POTHOS_TEST_BLOCK("/proxy/managed/tests", test_inception)
{
    auto env = Pothos::ProxyEnvironment::make("managed");

    //spawn server and connect
    auto serverHandle1 = env->findProxy("Pothos/RemoteServer").callProxy("new", "tcp://0.0.0.0");
    auto actualPort1 = serverHandle1.call<std::string>("getActualPort");
    auto clientHandle1 = env->findProxy("Pothos/RemoteClient").callProxy("new", "tcp://localhost:"+actualPort1);

    //create a remote environment
    auto &ios = clientHandle1.call<std::iostream &>("getIoStream");
    auto remoteEnv = Pothos::RemoteClient::makeEnvironment(ios, "managed");

    //now the remove env can make a new server
    //which can now be connected to locally
    auto serverHandle2 = remoteEnv->findProxy("Pothos/RemoteServer").callProxy("new", "tcp://0.0.0.0");
    auto actualPort2 = serverHandle2.call<std::string>("getActualPort");
    auto clientHandle2 = env->findProxy("Pothos/RemoteClient").callProxy("new", "tcp://localhost:"+actualPort2);
}

//! A thread to handle remote proxy requests
class HandlerRunnable: public Poco::Runnable
{
public:
    HandlerRunnable(Poco::Pipe &p0, Poco::Pipe &p1):
        p0(p0), p1(p1){}

private:
    Poco::Pipe &p0, &p1;
    void run(void)
    {
        //std::cout << "run start \n";
        Poco::PipeInputStream is(p0);
        Poco::PipeOutputStream os(p1);
        Pothos::RemoteHandler handler;
        handler.runHandler(is, os);
        //std::cout << "run exit \n";
    }
};

POTHOS_TEST_BLOCK("/proxy/managed/tests", test_remote)
{
    //tests remote proxy as well as managed:
    Poco::Pipe p0, p1;
    Poco::PipeInputStream is(p1);
    Poco::PipeOutputStream os(p0);

    Poco::Thread thread;
    HandlerRunnable runnable(p0, p1);
    thread.start(runnable);

    test_simple_runner(Pothos::RemoteClient::makeEnvironment(is, os, "managed"));
    thread.join();
}

POTHOS_TEST_BLOCK("/proxy/managed/tests", test_server)
{
    Pothos::RemoteServer server("tcp://0.0.0.0");
    Pothos::RemoteClient client("tcp://localhost:"+server.getActualPort());
    auto env = Pothos::RemoteClient::makeEnvironment(client.getIoStream(), "managed");
    std::cout << "Env peering address " << env->getPeeringAddress() << std::endl;
}

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
