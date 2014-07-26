// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Framework.hpp>
#include <Pothos/Proxy.hpp>
#include <Poco/Format.h>
#include <Poco/JSON/Object.h>
#include <iostream>

static void network_test_harness(const std::string &scheme, const bool serverIsSource)
{
    std::cout << Poco::format("network_test_harness: %s:// (serverIsSource? %s)",
        scheme, std::string(serverIsSource?"true":"false")) << std::endl;
    auto env = Pothos::ProxyEnvironment::make("managed")->findProxy("Pothos/BlockRegistry");

    //create server
    auto server_uri = Poco::format("%s://0.0.0.0", scheme);
    std::cout << "make server " << server_uri << std::endl;
    auto server = env.callProxy(
        (serverIsSource)?"/blocks/network_source":"/blocks/network_sink",
        server_uri, "BIND");

    //create client
    auto client_uri = Poco::format("%s://localhost:%s", scheme, server.call<std::string>("getActualPort"));
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

    //create a test plan
    Poco::JSON::Object::Ptr testPlan(new Poco::JSON::Object());
    testPlan->set("enableBuffers", true);
    testPlan->set("enableLabels", true);
    testPlan->set("enableMessages", true);
    //large and numerous payloads
    testPlan->set("minTrials", 100);
    testPlan->set("maxTrials", 200);
    testPlan->set("minSize", 512);
    testPlan->set("maxSize", 1048); //TODO FIXME see #7
    auto expected = feeder.callProxy("feedTestPlan", testPlan);

    //create tester topology
    std::cout << "Basic message test" << std::endl;
    {
        Pothos::Topology topology;
        topology.connect(source, 0, collector, 0);
        topology.connect(feeder, 0, sink, 0);
        topology.commit();
        POTHOS_TEST_TRUE(topology.waitInactive());
    }

    std::cout << "verifyTestPlan" << std::endl;
    collector.callVoid("verifyTestPlan", expected);

    //create tester topology -- tests for open close
    std::cout << "Open/close repeat test" << std::endl;
    for (size_t i = 0; i < 3; i++)
    {
        Pothos::Topology topology;
        topology.connect(source, 0, collector, 0);
        topology.connect(feeder, 0, sink, 0);
        topology.commit();
    }

    std::cout << "Done!\n" << std::endl;
}

POTHOS_TEST_BLOCK("/blocks/tests", test_network_blocks)
{
    network_test_harness("tcp", true);
    network_test_harness("tcp", false);
    network_test_harness("udt", true);
    network_test_harness("udt", false);
}
