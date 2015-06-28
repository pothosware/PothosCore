// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Framework.hpp>
#include <Pothos/Proxy.hpp>
#include <Poco/JSON/Object.h>
#include <iostream>

static void test_packet_blocks_with_mtu(const size_t mtu)
{
    std::cout << "testing MTU " << mtu << std::endl;

    auto env = Pothos::ProxyEnvironment::make("managed");
    auto registry = env->findProxy("Pothos/BlockRegistry");
    auto feeder = registry.callProxy("/blocks/feeder_source", "int");
    auto collector = registry.callProxy("/blocks/collector_sink", "int");

    auto s2p = registry.callProxy("/blocks/stream_to_packet");
    s2p.callVoid("setMTU", mtu);
    auto p2s = registry.callProxy("/blocks/packet_to_stream");

    //create a test plan
    Poco::JSON::Object::Ptr testPlan(new Poco::JSON::Object());
    testPlan->set("enableBuffers", true);
    testPlan->set("enableLabels", true);
    testPlan->set("enableMessages", true);
    auto expected = feeder.callProxy("feedTestPlan", testPlan);

    //run the topology
    {
        Pothos::Topology topology;
        topology.connect(feeder, 0, s2p, 0);
        topology.connect(s2p, 0, p2s, 0);
        topology.connect(p2s, 0, collector, 0);
        topology.commit();
        POTHOS_TEST_TRUE(topology.waitInactive());
    }

    collector.callVoid("verifyTestPlan", expected);
}

POTHOS_TEST_BLOCK("/blocks/tests", test_packet_blocks)
{
    test_packet_blocks_with_mtu(100); //medium
    test_packet_blocks_with_mtu(4096); //large
    test_packet_blocks_with_mtu(0); //unconstrained
}

POTHOS_TEST_BLOCK("/blocks/tests", test_packet_to_stream)
{
    auto env = Pothos::ProxyEnvironment::make("managed");
    auto registry = env->findProxy("Pothos/BlockRegistry");

    //create the blocks
    auto feeder = registry.callProxy("/blocks/feeder_source", "int");
    auto collector = registry.callProxy("/blocks/collector_sink", "int");
    auto p2s = registry.callProxy("/blocks/packet_to_stream");
    p2s.callVoid("setFrameStartId", "SOF0");
    p2s.callVoid("setFrameEndId", "EOF0");

    //create test data
    Pothos::Packet p0;
    p0.payload = Pothos::BufferChunk("int", 100);
    for (size_t i = 0; i < p0.payload.elements(); i++)
        p0.payload.as<int *>()[i] = std::rand();
    feeder.callVoid("feedPacket", p0);

    //create the topology
    Pothos::Topology topology;
    topology.connect(feeder, 0, p2s, 0);
    topology.connect(p2s, 0, collector, 0);
    topology.commit();
    POTHOS_TEST_TRUE(topology.waitInactive());

    //check the result
    const auto buffer = collector.call<Pothos::BufferChunk>("getBuffer");
    POTHOS_TEST_EQUAL(buffer.elements(), p0.payload.elements());
    POTHOS_TEST_EQUALA(buffer.as<const int *>(), p0.payload.as<const int *>(), p0.payload.elements());
    const auto labels = collector.call<std::vector<Pothos::Label>>("getLabels");
    POTHOS_TEST_EQUAL(labels.size(), 2);
    POTHOS_TEST_EQUAL(labels[0].id, "SOF0");
    POTHOS_TEST_EQUAL(labels[0].index, 0);
    POTHOS_TEST_EQUAL(labels[0].width, 1);
    POTHOS_TEST_EQUAL(labels[1].id, "EOF0");
    POTHOS_TEST_EQUAL(labels[1].index, p0.payload.elements()-1);
    POTHOS_TEST_EQUAL(labels[1].width, 1);
}
