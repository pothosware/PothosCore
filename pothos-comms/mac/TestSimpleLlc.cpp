// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Framework.hpp>
#include <Pothos/Proxy.hpp>
#include <Poco/JSON/Object.h>
#include <iostream>

POTHOS_TEST_BLOCK("/blocks/tests", test_simple_llc)
{
    auto env = Pothos::ProxyEnvironment::make("managed");
    auto registry = env->findProxy("Pothos/BlockRegistry");
    const uint8_t port = 123;

    //create side A test blocks
    auto feederA = registry.callProxy("/blocks/feeder_source", "uint8");
    feederA.callVoid("setName", "feederA");
    auto collectorA = registry.callProxy("/blocks/collector_sink", "uint8");
    collectorA.callVoid("setName", "collectorA");
    auto llcA = registry.callProxy("/blocks/simple_llc");
    llcA.callVoid("setName", "llcA");
    llcA.callVoid("setRecipient", 0xB); //sends to size B
    llcA.callVoid("setPort", port);
    auto macA = registry.callProxy("/blocks/simple_mac");
    macA.callVoid("setName", "macA");
    macA.callVoid("setMacId", 0xA);

    //create side B test blocks
    auto feederB = registry.callProxy("/blocks/feeder_source", "uint8");
    feederB.callVoid("setName", "feederB");
    auto collectorB = registry.callProxy("/blocks/collector_sink", "uint8");
    collectorB.callVoid("setName", "collectorB");
    auto llcB = registry.callProxy("/blocks/simple_llc");
    llcB.callVoid("setName", "llcB");
    llcB.callVoid("setRecipient", 0xA); //sends to size A
    llcB.callVoid("setPort", port);
    auto macB = registry.callProxy("/blocks/simple_mac");
    macB.callVoid("setName", "macB");
    macB.callVoid("setMacId", 0xB);

    //create a test packet
    Pothos::Packet pktA2B;
    pktA2B.payload = Pothos::BufferChunk("uint8", 100);
    for (size_t i = 0; i < pktA2B.payload.elements(); i++)
        pktA2B.payload.as<unsigned char *>()[i] = std::rand() & 0xff;
    feederA.callVoid("feedPacket", pktA2B);

    //create a test packet
    Pothos::Packet pktB2A;
    pktB2A.payload = Pothos::BufferChunk("uint8", 100);
    for (size_t i = 0; i < pktB2A.payload.elements(); i++)
        pktB2A.payload.as<unsigned char *>()[i] = std::rand() & 0xff;
    feederB.callVoid("feedPacket", pktB2A);

    //setup the topology
    Pothos::Topology topology;

    //connect collector/feeder A to LLC A
    topology.connect(feederA, 0, llcA, "dataIn");
    topology.connect(llcA, "dataOut", collectorA, 0);

    //connect MAC A to LLC A
    topology.connect(llcA, "macOut", macA, "macIn");
    topology.connect(macA, "macOut", llcA, "macIn");

    //connect collector/feeder B to LLC B
    topology.connect(feederB, 0, llcB, "dataIn");
    topology.connect(llcB, "dataOut", collectorB, 0);

    //connect MAC B to LLC B
    topology.connect(llcB, "macOut", macB, "macIn");
    topology.connect(macB, "macOut", llcB, "macIn");

    //connect MAC A to MAC B
    topology.connect(macA, "phyOut", macB, "phyIn");
    topology.connect(macB, "phyOut", macA, "phyIn");

    //run the design
    topology.commit();
    POTHOS_TEST_TRUE(topology.waitInactive());
    //std::cout << topology.queryJSONStats() << std::endl;

    //check side A
    POTHOS_TEST_EQUAL(macA.call<unsigned long long>("getErrorCount"), 0);
    const auto packetsA = collectorA.call<std::vector<Pothos::Packet>>("getPackets");
    POTHOS_TEST_EQUAL(packetsA.size(), 1);
    const auto pktOutA0 = packetsA.at(0);
    POTHOS_TEST_EQUAL(pktB2A.payload.dtype, pktOutA0.payload.dtype);
    POTHOS_TEST_EQUAL(pktB2A.payload.elements(), pktOutA0.payload.elements());
    POTHOS_TEST_EQUALA(pktB2A.payload.as<const unsigned char *>(),
        pktOutA0.payload.as<const unsigned char *>(), pktOutA0.payload.elements());

    //check side B
    POTHOS_TEST_EQUAL(macB.call<unsigned long long>("getErrorCount"), 0);
    const auto packetsB = collectorB.call<std::vector<Pothos::Packet>>("getPackets");
    POTHOS_TEST_EQUAL(packetsB.size(), 1);
    const auto pktOutB0 = packetsB.at(0);
    POTHOS_TEST_EQUAL(pktA2B.payload.dtype, pktOutB0.payload.dtype);
    POTHOS_TEST_EQUAL(pktA2B.payload.elements(), pktOutB0.payload.elements());
    POTHOS_TEST_EQUALA(pktA2B.payload.as<const unsigned char *>(),
        pktOutB0.payload.as<const unsigned char *>(), pktOutB0.payload.elements());
}

POTHOS_TEST_BLOCK("/blocks/tests", test_simple_llc_harsh)
{
    auto env = Pothos::ProxyEnvironment::make("managed");
    auto registry = env->findProxy("Pothos/BlockRegistry");
    const uint8_t port = 123;

    //create side A test blocks
    auto feederA = registry.callProxy("/blocks/feeder_source", "uint8");
    feederA.callVoid("setName", "feederA");
    auto collectorA = registry.callProxy("/blocks/collector_sink", "uint8");
    collectorA.callVoid("setName", "collectorA");
    auto llcA = registry.callProxy("/blocks/simple_llc");
    llcA.callVoid("setName", "llcA");
    llcA.callVoid("setRecipient", 0xB); //sends to size B
    llcA.callVoid("setPort", port);
    auto macA = registry.callProxy("/blocks/simple_mac");
    macA.callVoid("setName", "macA");
    macA.callVoid("setMacId", 0xA);

    //create side B test blocks
    auto feederB = registry.callProxy("/blocks/feeder_source", "uint8");
    feederB.callVoid("setName", "feederB");
    auto collectorB = registry.callProxy("/blocks/collector_sink", "uint8");
    collectorB.callVoid("setName", "collectorB");
    auto llcB = registry.callProxy("/blocks/simple_llc");
    llcB.callVoid("setName", "llcB");
    llcB.callVoid("setRecipient", 0xA); //sends to size A
    llcB.callVoid("setPort", port);
    auto macB = registry.callProxy("/blocks/simple_mac");
    macB.callVoid("setName", "macB");
    macB.callVoid("setMacId", 0xB);

    //create dropper blocks for connection
    const double dropChance = 0.05; //chance of dropping out of 1.0
    auto dropperA2B = registry.callProxy("/blocks/sporadic_dropper");
    dropperA2B.callVoid("setProbability", dropChance); //chance of drop
    auto dropperB2A = registry.callProxy("/blocks/sporadic_dropper");
    dropperB2A.callVoid("setProbability", dropChance); //chance of drop

    //create the test plan for both feeders
    Poco::JSON::Object::Ptr testPlan(new Poco::JSON::Object());
    testPlan->set("enablePackets", true);
    testPlan->set("minBuffers", 500); //many packets
    testPlan->set("maxBuffers", 500); //many packets
    auto expectedA2B = feederA.callProxy("feedTestPlan", testPlan);
    auto expectedB2A = feederB.callProxy("feedTestPlan", testPlan);

    //setup the topology
    Pothos::Topology topology;

    //connect collector/feeder A to LLC A
    topology.connect(feederA, 0, llcA, "dataIn");
    topology.connect(llcA, "dataOut", collectorA, 0);

    //connect MAC A to LLC A
    topology.connect(llcA, "macOut", macA, "macIn");
    topology.connect(macA, "macOut", llcA, "macIn");

    //connect collector/feeder B to LLC B
    topology.connect(feederB, 0, llcB, "dataIn");
    topology.connect(llcB, "dataOut", collectorB, 0);

    //connect MAC B to LLC B
    topology.connect(llcB, "macOut", macB, "macIn");
    topology.connect(macB, "macOut", llcB, "macIn");

    //connect MAC A to MAC B through dropper blocks
    topology.connect(macA, "phyOut", dropperA2B, 0);
    topology.connect(dropperA2B, 0, macB, "phyIn");
    topology.connect(macB, "phyOut", dropperB2A, 0);
    topology.connect(dropperB2A, 0, macA, "phyIn");

    //run the design
    topology.commit();
    POTHOS_TEST_TRUE(topology.waitInactive(0.5, 0.0));
    //std::cout << topology.queryJSONStats() << std::endl;

    //check the results
    std::cout << "llcA resend count " << llcA.call<unsigned long long>("getResendCount") << std::endl;
    std::cout << "llcB resend count " << llcB.call<unsigned long long>("getResendCount") << std::endl;
    POTHOS_TEST_EQUAL(llcA.call<unsigned long long>("getExpiredCount"), 0);
    POTHOS_TEST_EQUAL(llcB.call<unsigned long long>("getExpiredCount"), 0);
    collectorA.callVoid("verifyTestPlan", expectedB2A);
    collectorB.callVoid("verifyTestPlan", expectedA2B);
}
