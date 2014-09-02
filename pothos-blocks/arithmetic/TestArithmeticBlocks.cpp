// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Framework.hpp>
#include <Pothos/Proxy.hpp>
#include <iostream>

POTHOS_TEST_BLOCK("/blocks/tests", test_arithmetic_add)
{
    auto registry = Pothos::ProxyEnvironment::make("managed")->findProxy("Pothos/BlockRegistry");

    auto feeder0 = registry.callProxy("/blocks/feeder_source", "int");
    auto feeder1 = registry.callProxy("/blocks/feeder_source", "int");
    auto adder = registry.callProxy("/blocks/arithmetic", "int", "ADD");
    auto collector = registry.callProxy("/blocks/collector_sink", "int");

    //load feeder blocks
    auto b0 = Pothos::BufferChunk(10*sizeof(int));
    auto p0 = b0.as<int *>();
    for (size_t i = 0; i < 10; i++) p0[i] = i;
    feeder0.callProxy("feedBuffer", b0);

    auto b1 = Pothos::BufferChunk(10*sizeof(int));
    auto p1 = b1.as<int *>();
    for (size_t i = 0; i < 10; i++) p1[i] = i+10;
    feeder1.callProxy("feedBuffer", b1);

    //run the topology
    {
        Pothos::Topology topology;
        topology.connect(feeder0, 0, adder, 0);
        topology.connect(feeder1, 0, adder, 1);
        topology.connect(adder, 0, collector, 0);
        topology.commit();
        POTHOS_TEST_TRUE(topology.waitInactive());
    }

    //check the collector
    auto buff = collector.call<Pothos::BufferChunk>("getBuffer");
    POTHOS_TEST_EQUAL(buff.length, 10*sizeof(int));
    auto pb = buff.as<const int *>();
    //for (int i = 0; i < 10; i++) std::cout << i << " " << pb[i] << std::endl;
    for (int i = 0; i < 10; i++) POTHOS_TEST_EQUAL(pb[i], i+i+10);
}

POTHOS_TEST_BLOCK("/blocks/tests", test_arithmetic_feedback)
{
    auto registry = Pothos::ProxyEnvironment::make("managed")->findProxy("Pothos/BlockRegistry");

    auto feeder = registry.callProxy("/blocks/feeder_source", "int");
    auto adder = registry.callProxy("/blocks/arithmetic", "int", "ADD");
    auto collector = registry.callProxy("/blocks/collector_sink", "int");

    //adder has a preload on input1 for feedback loop
    std::vector<size_t> preload(2, 0); preload[1] = 1;
    adder.callVoid("setPreload", preload);

    //load feeder block
    auto b0 = Pothos::BufferChunk(10*sizeof(int));
    auto p0 = b0.as<int *>();
    for (size_t i = 0; i < 10; i++) p0[i] = i;
    feeder.callProxy("feedBuffer", b0);

    //run the topology
    {
        Pothos::Topology topology;
        topology.connect(feeder, 0, adder, 0);
        topology.connect(adder, 0, adder, 1);
        topology.connect(adder, 0, collector, 0);
        topology.commit();
        POTHOS_TEST_TRUE(topology.waitInactive());
    }

    //check the collector
    auto buff = collector.call<Pothos::BufferChunk>("getBuffer");
    POTHOS_TEST_EQUAL(buff.length, 10*sizeof(int));
    auto pb = buff.as<const int *>();
    //for (int i = 0; i < 10; i++) std::cout << i << " " << pb[i] << std::endl;
    int last = 0;
    for (int i = 0; i < 10; i++)
    {
        last = i + last;
        POTHOS_TEST_EQUAL(pb[i], last);
    }
}

POTHOS_TEST_BLOCK("/blocks/tests", test_inline_buffer)
{
    auto registry = Pothos::ProxyEnvironment::make("managed")->findProxy("Pothos/BlockRegistry");

    auto feeder0 = registry.callProxy("/blocks/feeder_source", "int");
    auto feeder1 = registry.callProxy("/blocks/feeder_source", "int");
    auto copier = registry.callProxy("/blocks/copier");
    auto adder = registry.callProxy("/blocks/arithmetic", "int", "ADD");
    auto collector = registry.callProxy("/blocks/collector_sink", "int");

    //load feeder blocks
    const auto numElems = 4000;
    auto b0 = Pothos::BufferChunk(numElems*sizeof(int));
    auto p0 = b0.as<int *>();
    for (size_t i = 0; i < numElems; i++) p0[i] = i;
    feeder0.callProxy("feedBuffer", b0);

    auto b1 = Pothos::BufferChunk(numElems*sizeof(int));
    auto p1 = b1.as<int *>();
    for (size_t i = 0; i < numElems; i++) p1[i] = i+numElems;
    //feeder1.callProxy("feedBuffer", b1);

    //run the topology
    {
        Pothos::Topology topology;
        topology.connect(feeder0, 0, copier, 0); //copier before adder ensures framework provided buffers
        topology.connect(copier, 0, adder, 0);
        topology.connect(feeder1, 0, adder, 1);
        topology.connect(adder, 0, collector, 0);
        topology.commit();
        POTHOS_TEST_TRUE(topology.waitInactive());
        feeder1.callProxy("feedBuffer", b1); //ensure that the buffer will be inlined by forcing processing on a non port 0 message
        POTHOS_TEST_TRUE(topology.waitInactive());
    }

    //check the collector
    auto buff = collector.call<Pothos::BufferChunk>("getBuffer");
    POTHOS_TEST_EQUAL(buff.length, numElems*sizeof(int));
    auto pb = buff.as<const int *>();
    //for (int i = 0; i < numElems; i++) std::cout << i << " " << pb[i] << std::endl;
    for (int i = 0; i < numElems; i++) POTHOS_TEST_EQUAL(pb[i], i+i+numElems);

    auto numInlines = adder.call<size_t>("getNumInlineBuffers");
    std::cout << "NumInlineBuffers " << numInlines << std::endl;
    POTHOS_TEST_TRUE(numInlines > 0);
}
