// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Framework.hpp>
#include <Pothos/Proxy.hpp>
#include <iostream>
#include <complex>

POTHOS_TEST_BLOCK("/blocks/tests", test_preamble_correlator)
{
    auto env = Pothos::ProxyEnvironment::make("managed");
    auto registry = env->findProxy("Pothos/BlockRegistry");

    auto feeder = registry.callProxy("/blocks/feeder_source", "unsigned char");
    auto correlator = registry.callProxy("/blocks/preamble_correlator");
    auto collector = registry.callProxy("/blocks/collector_sink", "unsigned char");

    static const unsigned char preambleD[] = {0, 1, 1, 1, 1, 0};
    std::vector<unsigned char> preamble(preambleD, preambleD+6);
    size_t testLength = 10 + preamble.size();
    size_t preambleIndex = 4;

    correlator.callProxy("setPreamble", preamble);
    correlator.callProxy("setThreshold", 0);

    //load feeder blocks
    auto b0 = Pothos::BufferChunk(testLength * sizeof(unsigned char));
    auto p0 = b0.as<unsigned char *>();
    for (size_t i = 0; i < testLength; i++) p0[i] = i % 2;
    for (size_t i = 0; i < preamble.size(); i++) p0[i + preambleIndex] = preamble[i];
    feeder.callProxy("feedBuffer", b0);

    //run the topology
    {
        Pothos::Topology topology;
        topology.connect(feeder, 0, correlator, 0);
        topology.connect(correlator, 0, collector, 0);
        topology.commit();
        POTHOS_TEST_TRUE(topology.waitInactive());
    }

    //check the collector buffer matches input
    auto buff = collector.call<Pothos::BufferChunk>("getBuffer");
    auto pb = buff.as<const unsigned char *>();
    for (size_t i = 0; i < testLength; i++) POTHOS_TEST_EQUAL(pb[i], p0[i]);

    //check for the preamble label
    auto labels = collector.call<std::vector<Pothos::Label>>("getLabels");
    POTHOS_TEST_EQUAL(labels.size(), 1);
    for (auto label : labels) POTHOS_TEST_EQUAL(label.index, preambleIndex + preamble.size());
}
