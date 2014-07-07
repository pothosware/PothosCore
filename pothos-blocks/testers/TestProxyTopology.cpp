// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Framework.hpp>
#include <Pothos/Proxy.hpp>
#include <Pothos/Remote.hpp>
#include <iostream>

POTHOS_TEST_BLOCK("/blocks/tests", test_proxy_topology)
{
    auto env = Pothos::ProxyEnvironment::make("managed");
    auto registry = env->findProxy("Pothos/BlockRegistry");
    auto feeder = registry.callProxy("/blocks/feeder_source", "int");
    auto collector = registry.callProxy("/blocks/collector_sink", "int");

    //feed some msgs
    std::cout << "give messages to the feeder\n";
    feeder.callVoid("feedMessage", Pothos::Object("msg0"));
    feeder.callVoid("feedMessage", Pothos::Object("msg1"));

    //run the topology
    std::cout << "run the topology\n";
    {
        auto topology = env->findProxy("Pothos/Topology").callProxy("new");
        topology.callVoid("connect", feeder, "0", collector, "0");
        topology.callVoid("commit");
        POTHOS_TEST_TRUE(topology.call<bool>("waitInactive"));
    }

    //check msgs
    std::cout << "check the collector for msgs\n";
    auto msgs = collector.call<std::vector<Pothos::Object>>("getMessages");
    POTHOS_TEST_EQUAL(msgs.size(), 2);
    POTHOS_TEST_EQUAL(msgs[0].extract<std::string>(), "msg0");
    POTHOS_TEST_EQUAL(msgs[1].extract<std::string>(), "msg1");

    std::cout << "done!\n";
}

//create subtopology as per https://github.com/pothosware/pothos-library/issues/44
static Pothos::Topology* makeForwardingTopology(void)
{
    auto env = Pothos::ProxyEnvironment::make("managed");
    auto registry = env->findProxy("Pothos/BlockRegistry");
    auto forwarder = registry.callProxy("/blocks/forwarder");
    auto t = new Pothos::Topology();
    t->connect(t, "0", forwarder, "0");
    t->connect(forwarder, "0", t, "0");
    return t;
}

static Pothos::BlockRegistry registerAdd(
    "/blocks/tests/forwarder_topology", &makeForwardingTopology);

POTHOS_TEST_BLOCK("/blocks/tests", test_proxy_subtopology)
{
    //spawn a server and client
    std::cout << "create proxy server\n";
    Pothos::RemoteServer server("tcp://0.0.0.0");
    Pothos::RemoteClient client("tcp://localhost:"+server.getActualPort());
    auto env = Pothos::ProxyEnvironment::make("managed");
    auto envRemote = client.makeEnvironment("managed");

    auto registry = env->findProxy("Pothos/BlockRegistry");
    auto registryRemote = envRemote->findProxy("Pothos/BlockRegistry");

    auto feeder = registry.callProxy("/blocks/feeder_source", "int");
    auto collector = registry.callProxy("/blocks/collector_sink", "int");
    std::cout << "make the remote subtopology\n";
    auto forwarder = registryRemote.callProxy("/blocks/tests/forwarder_topology");

    //feed some msgs
    std::cout << "give messages to the feeder\n";
    feeder.callVoid("feedMessage", Pothos::Object("msg0"));
    feeder.callVoid("feedMessage", Pothos::Object("msg1"));

    //run the topology
    std::cout << "run the topology\n";
    {
        auto topology = Pothos::Topology();
        topology.connect(feeder, "0", forwarder, "0");
        topology.connect(forwarder, "0", collector, "0");
        topology.commit();
        POTHOS_TEST_TRUE(topology.waitInactive());
    }

    //check msgs
    std::cout << "check the collector for msgs\n";
    auto msgs = collector.call<std::vector<Pothos::Object>>("getMessages");
    POTHOS_TEST_EQUAL(msgs.size(), 2);
    POTHOS_TEST_EQUAL(msgs[0].extract<std::string>(), "msg0");
    POTHOS_TEST_EQUAL(msgs[1].extract<std::string>(), "msg1");

    std::cout << "done!\n";
}
