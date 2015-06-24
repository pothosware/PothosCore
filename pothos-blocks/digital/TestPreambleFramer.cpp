// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Framework.hpp>
#include <Pothos/Proxy.hpp>
#include <iostream>
#include <complex>

POTHOS_TEST_BLOCK("/blocks/tests", test_preamble_framer)
{
    auto env = Pothos::ProxyEnvironment::make("managed");
    auto registry = env->findProxy("Pothos/BlockRegistry");

    auto feeder = registry.callProxy("/blocks/feeder_source", "unsigned char");
    auto framer = registry.callProxy("/blocks/preamble_framer");
    auto collector = registry.callProxy("/blocks/collector_sink", "unsigned char");

    static const unsigned char preambleD[] = {0, 1, 1, 1, 1, 0};
    std::vector<unsigned char> preamble(preambleD, preambleD+6);
    size_t testLength = 10;
    size_t preambleIndex = 4;

    framer.callProxy("setPreamble", preamble);
    framer.callProxy("setFrameStartId", "myFrameStart");

    //load feeder blocks
    auto b0 = Pothos::BufferChunk(testLength * sizeof(unsigned char));
    auto p0 = b0.as<unsigned char *>();
    for (size_t i = 0; i < testLength; i++) p0[i] = i % 2;
    feeder.callVoid("feedBuffer", b0);
    feeder.callVoid("feedLabel", Pothos::Label("myFrameStart", Pothos::Object(), preambleIndex));

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
    POTHOS_TEST_EQUAL(buff.elements(), testLength + preamble.size());
    auto pb = buff.as<const unsigned char *>();
    POTHOS_TEST_EQUALA(pb, p0, preambleIndex);
    POTHOS_TEST_EQUALA(pb+preambleIndex, preamble.data(), preamble.size());
    POTHOS_TEST_EQUALA(pb+preambleIndex+preamble.size(), p0+preambleIndex, testLength-preambleIndex);
}
