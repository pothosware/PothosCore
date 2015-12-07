// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Framework.hpp>
#include <Pothos/Proxy.hpp>
#include <Poco/Format.h>
#include <Poco/JSON/Object.h>
#include <Pothos/Util/Network.hpp>
#include <iostream>

static void network_test_harness(const std::string &scheme, const bool serverIsSource)
{
    std::cout << Poco::format("network_test_harness: %s:// (serverIsSource? %s)",
        scheme, std::string(serverIsSource?"true":"false")) << std::endl;
    auto env = Pothos::ProxyEnvironment::make("managed")->findProxy("Pothos/BlockRegistry");

    //create server
    auto server_uri = Poco::format("%s://%s", scheme, Pothos::Util::getWildcardAddr());
    std::cout << "make server " << server_uri << std::endl;
    auto server = env.callProxy(
        (serverIsSource)?"/blocks/network_source":"/blocks/network_sink",
        server_uri, "BIND");

    //create client
    auto client_uri = Poco::format("%s://%s", scheme, Pothos::Util::getLoopbackAddr(server.call<std::string>("getActualPort")));
    std::cout << "make client " << client_uri << std::endl;
    auto client = env.callProxy(
        (serverIsSource)?"/blocks/network_sink":"/blocks/network_source",
        client_uri, "CONNECT");

    //who is the source/sink?
    auto source = (serverIsSource)? server : client;
    auto sink = (serverIsSource)? client : server;

    //tester blocks
    auto feeder = env.callProxy("/blocks/feeder_source", "int");
    auto collector = env.callProxy("/blocks/collector_sink", "int");

    //create tester topology -- tests for open close
    std::cout << "Open/close repeat test" << std::endl;
    for (size_t i = 0; i < 3; i++)
    {
        Pothos::Topology topology;
        topology.connect(source, 0, collector, 0);
        topology.connect(feeder, 0, sink, 0);
        topology.commit();
    }

    //create tester topology
    Pothos::Topology topology;
    topology.connect(source, 0, collector, 0);
    topology.connect(feeder, 0, sink, 0);

    //create the testplan with large and numerous payloads
    Poco::JSON::Object::Ptr testPlan(new Poco::JSON::Object());
    testPlan->set("enableLabels", true);
    testPlan->set("enableMessages", true);
    testPlan->set("minTrials", 100);
    testPlan->set("maxTrials", 200);
    testPlan->set("minSize", 512);
    testPlan->set("maxSize", 1048*8);

    //test buffers with labels and messages
    std::cout << "Buffer based test" << std::endl;
    testPlan->set("enablePackets", false);
    testPlan->set("enableBuffers", true);
    auto expected = feeder.callProxy("feedTestPlan", testPlan);
    topology.commit();
    POTHOS_TEST_TRUE(topology.waitInactive());
    collector.callVoid("verifyTestPlan", expected);

    //work around bug in poco 1.6.1:
    //https://github.com/pocoproject/poco/issues/933
    testPlan->remove("enablePackets");
    testPlan->remove("enableBuffers");

    //test packets with labels and messages
    std::cout << "Packet based test" << std::endl;
    testPlan->set("enablePackets", true);
    testPlan->set("enableBuffers", false);
    expected = feeder.callProxy("feedTestPlan", testPlan);
    topology.commit();
    POTHOS_TEST_TRUE(topology.waitInactive());
    collector.callVoid("verifyTestPlan", expected);

    std::cout << "Done!\n" << std::endl;
}

POTHOS_TEST_BLOCK("/blocks/tests", test_network_blocks)
{
    network_test_harness("tcp", true);
    network_test_harness("tcp", false);
    //network_test_harness("udt", true);
    //network_test_harness("udt", false);
}
