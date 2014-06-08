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
    auto feeder = registry.callProxy("/blocks/sources/feeder_source", "int");
    auto collector = registry.callProxy("/blocks/sinks/collector_sink", "int");

    //feed some msgs
    std::cout << "give messages to the feeder\n";
    feeder.call("feedMessage", Pothos::Object("msg0"));
    feeder.call("feedMessage", Pothos::Object("msg1"));

    //run the topology
    std::cout << "run the topology\n";
    {
        auto topology = env->findProxy("Pothos/Topology").callProxy("new");
        topology.call("connect", feeder, "0", collector, "0");
        topology.call("commit");
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

    auto feeder = registry.callProxy("/blocks/sources/feeder_source", "int");
    auto collector = registry.callProxy("/blocks/sinks/collector_sink", "int");
    auto forwarder = registryRemote.callProxy("/blocks/misc/forwarder", "int");

    //feed some msgs
    std::cout << "give messages to the feeder\n";
    feeder.callProxy("feedMessage", Pothos::Object("msg0"));
    feeder.callProxy("feedMessage", Pothos::Object("msg1"));

    //connect subtopology that just forwards
    std::cout << "connect the remote subtopology\n";
    auto subtopology = envRemote->findProxy("Pothos/Topology").callProxy("new");
    subtopology.call("connect", subtopology, "0", forwarder, "0");
    subtopology.call("connect", forwarder, "0", subtopology, "0");

    //run the topology
    std::cout << "run the topology\n";
    {
        auto topology = env->findProxy("Pothos/Topology").callProxy("new");
        POTHOS_TEST_TRUE(subtopology.call<std::string>("uid") != topology.call<std::string>("uid"));
        topology.call("connect", feeder, "0", subtopology, "0");
        topology.call("connect", subtopology, "0", collector, "0");
        topology.call("commit");
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
