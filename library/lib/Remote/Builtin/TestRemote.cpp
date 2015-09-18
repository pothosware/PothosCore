// Copyright (c) 2013-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Plugin.hpp>
#include <Pothos/Proxy.hpp>
#include <Pothos/Remote.hpp>
#include <Pothos/Managed.hpp>
#include <Poco/Pipe.h>
#include <Poco/PipeStream.h>
#include <Poco/URI.h>
#include <iostream>
#include <future>
#include <thread>
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
    Pothos::ManagedClass::unload("SuperBar");
    Pothos::ManagedClass::unload("SuperFoo");
}

POTHOS_TEST_BLOCK("/proxy/remote/tests", test_inception)
{
    auto env = Pothos::ProxyEnvironment::make("managed");

    //spawn server and connect
    auto serverHandle1 = env->findProxy("Pothos/RemoteServer").callProxy("new", "tcp://[::]");
    auto actualPort1 = serverHandle1.call<std::string>("getActualPort");
    auto clientHandle1 = env->findProxy("Pothos/RemoteClient").callProxy("new", "tcp://[::1]:"+actualPort1);

    //create a remote environment
    auto &ios = clientHandle1.call<std::iostream &>("getIoStream");
    auto remoteEnv = Pothos::RemoteClient::makeEnvironment(ios, "managed");

    //now the remove env can make a new server
    //which can now be connected to locally
    auto serverHandle2 = remoteEnv->findProxy("Pothos/RemoteServer").callProxy("new", "tcp://[::]");
    auto actualPort2 = serverHandle2.call<std::string>("getActualPort");
    auto clientHandle2 = env->findProxy("Pothos/RemoteClient").callProxy("new", "tcp://[::1]:"+actualPort2);
}

//! A thread to handle remote proxy requests
static void runRemoteProxy(Poco::Pipe &p0, Poco::Pipe &p1)
{
    //std::cout << "run start \n";
    Poco::PipeInputStream is(p0);
    Poco::PipeOutputStream os(p1);
    Pothos::RemoteHandler handler;
    handler.runHandler(is, os);
    //std::cout << "run exit \n";
}

POTHOS_TEST_BLOCK("/proxy/remote/tests", test_remote)
{
    //check that the test runs locally first
    test_simple_runner(Pothos::ProxyEnvironment::make("managed"));

    //tests remote proxy as well as managed:
    Poco::Pipe p0, p1;
    Poco::PipeInputStream is(p1);
    Poco::PipeOutputStream os(p0);

    std::thread t0(&runRemoteProxy, std::ref(p0), std::ref(p1));

    test_simple_runner(Pothos::RemoteClient::makeEnvironment(is, os, "managed"));
    t0.join();
}

POTHOS_TEST_BLOCK("/proxy/remote/tests", test_server)
{
    Pothos::RemoteServer server("tcp://[::]");
    Pothos::RemoteClient client("tcp://[::1]:"+server.getActualPort());
    auto env = Pothos::RemoteClient::makeEnvironment(client.getIoStream(), "managed");
    std::cout << "Env peering address " << env->getPeeringAddress() << std::endl;
}

struct EchoTester
{
    static int echo(int x)
    {
        return x;
    }
};

static int callRemoteEcho(const Pothos::ProxyEnvironment::Sptr &env, int x)
{
    return env->findProxy("EchoTester").call<int>("echo", x);
}

POTHOS_TEST_BLOCK("/proxy/remote/tests", test_multithread_safe)
{
    Pothos::ManagedClass()
        .registerClass<EchoTester>()
        .registerStaticMethod(POTHOS_FCN_TUPLE(EchoTester, echo))
        .commit("EchoTester");
    Poco::Pipe p0, p1;
    Poco::PipeInputStream is(p1);
    Poco::PipeOutputStream os(p0);
    std::thread t0(&runRemoteProxy, std::ref(p0), std::ref(p1));
    {
        auto env = Pothos::RemoteClient::makeEnvironment(is, os, "managed");

        //create futures and expected results
        std::vector<std::future<int>> futures;
        std::vector<int> expected;
        for (size_t i = 0; i < 100; i++)
        {
            expected.push_back(std::rand());
            futures.push_back(std::async(std::launch::async, &callRemoteEcho, env, expected.back()));
        }

        //check results
        for (size_t i = 0; i < expected.size(); i++)
        {
            const auto val = futures[i].get();
            POTHOS_TEST_EQUAL(expected[i], val);
        }
    }

    t0.join();

    //runtime registration does not associate the module
    //therefore to be safe, we unregister these classes now
    Pothos::ManagedClass::unload("EchoTester");
}
