// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Framework.hpp>
#include <Pothos/Proxy.hpp>
#include <iostream>
#include <complex>

POTHOS_TEST_BLOCK("/comms/tests", test_preamble_framer)
{
    auto env = Pothos::ProxyEnvironment::make("managed");
    auto registry = env->findProxy("Pothos/BlockRegistry");

    auto feeder = registry.callProxy("/blocks/feeder_source", "unsigned char");
    auto framer = registry.callProxy("/comms/preamble_framer");
    auto collector = registry.callProxy("/blocks/collector_sink", "unsigned char");

    static const unsigned char preambleD[] = {0, 1, 1, 1, 1, 0};
    std::vector<unsigned char> preamble(preambleD, preambleD+6);
    size_t testLength = 40;
    size_t startIndex = 5;
    size_t endIndex = 33;
    size_t paddingSize = 13;

    framer.callProxy("setPreamble", preamble);
    framer.callProxy("setFrameStartId", "myFrameStart");
    framer.callProxy("setFrameEndId", "myFrameEnd");
    framer.callProxy("setPaddingSize", paddingSize);

    //load feeder blocks
    auto b0 = Pothos::BufferChunk(typeid(unsigned char), testLength);
    auto p0 = b0.as<unsigned char *>();
    for (size_t i = 0; i < testLength; i++) p0[i] = i % 2;
    feeder.callVoid("feedBuffer", b0);
    feeder.callVoid("feedLabel", Pothos::Label("myFrameStart", Pothos::Object(), startIndex));
    feeder.callVoid("feedLabel", Pothos::Label("myFrameEnd", Pothos::Object(), endIndex));

    //run the topology
    {
        Pothos::Topology topology;
        topology.connect(feeder, 0, framer, 0);
        topology.connect(framer, 0, collector, 0);
        topology.commit();
        POTHOS_TEST_TRUE(topology.waitInactive());
    }

    //check the collector buffer matches input with preamble inserted
    auto buff = collector.call<Pothos::BufferChunk>("getBuffer");
    POTHOS_TEST_EQUAL(buff.elements(), testLength + preamble.size() + paddingSize);
    auto pb = buff.as<const unsigned char *>();
    POTHOS_TEST_EQUALA(pb, p0, startIndex); //check data before frame
    POTHOS_TEST_EQUALA(pb+startIndex, preamble.data(), preamble.size()); //check preamble insertion
    POTHOS_TEST_EQUALA(pb+startIndex+preamble.size(), p0+startIndex, endIndex-startIndex+1); //check frame
    POTHOS_TEST_EQUALA(pb+endIndex+preamble.size()+paddingSize+1, p0+endIndex+1, testLength-(endIndex+1)); //check data after frame

    //check the label positions
    auto labels = collector.call<std::vector<Pothos::Label>>("getLabels");
    POTHOS_TEST_EQUAL(labels.size(), 2);
    POTHOS_TEST_EQUAL(labels[0].id, "myFrameStart");
    POTHOS_TEST_EQUAL(labels[0].index, startIndex);
    POTHOS_TEST_EQUAL(labels[1].id, "myFrameEnd");
    POTHOS_TEST_EQUAL(labels[1].index, endIndex+preamble.size()+paddingSize);
}
