// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Framework.hpp>
#include <Pothos/Proxy.hpp>
#include <Pothos/Remote.hpp>
#include <Poco/JSON/Object.h>
#include <iostream>

POTHOS_TEST_BLOCK("/blocks/tests", test_framer_to_correlator)
{
    auto env = Pothos::ProxyEnvironment::make("managed");
    auto registry = env->findProxy("Pothos/BlockRegistry");

    auto feeder = registry.callProxy("/blocks/feeder_source", "uint8");
    auto s2p = registry.callProxy("/blocks/stream_to_packet");
    auto generator = registry.callProxy("/blocks/packet_to_stream");
    auto framer = registry.callProxy("/blocks/preamble_framer");
    auto correlator = registry.callProxy("/blocks/preamble_correlator");
    auto deframer = registry.callProxy("/blocks/label_deframer");
    auto p2s = registry.callProxy("/blocks/packet_to_stream");
    auto collector = registry.callProxy("/blocks/collector_sink", "uint8");

    //configuration constants
    const size_t mtu = 107;
    const std::string txFrameStartId = "txFrameStart";
    const std::string rxFrameStartId = "rxFrameStart";
    const size_t maxValue = 1;
    std::vector<unsigned char> preamble;
    for (size_t i = 0; i < 32; i++) preamble.push_back(std::rand() % (maxValue+1));

    //configure
    generator.callVoid("setFrameStartId", txFrameStartId);
    generator.callVoid("setName", "frameGenerator");
    framer.callVoid("setPreamble", preamble);
    framer.callVoid("setFrameStartId", txFrameStartId);
    correlator.callVoid("setPreamble", preamble);
    correlator.callVoid("setThreshold", 0); //expect perfect match
    correlator.callVoid("setFrameStartId", rxFrameStartId);
    deframer.callVoid("setFrameStartId", rxFrameStartId);
    deframer.callVoid("setMTU", mtu);

    //create a test plan
    Poco::JSON::Object::Ptr testPlan(new Poco::JSON::Object());
    testPlan->set("enableBuffers", true);
    testPlan->set("minValue", 0);
    testPlan->set("maxValue", maxValue);
    testPlan->set("minBufferSize", mtu);
    testPlan->set("maxBufferSize", mtu);
    auto expected = feeder.callProxy("feedTestPlan", testPlan);

    //because of correlation window, pad feeder to flush through last message
    feeder.callVoid("feedBuffer", Pothos::BufferChunk(preamble.size()));

    //create tester topology
    {
        Pothos::Topology topology;
        topology.connect(feeder, 0, s2p, 0);
        topology.connect(s2p, 0, generator, 0);
        topology.connect(generator, 0, framer, 0);
        topology.connect(framer, 0, correlator, 0);
        topology.connect(correlator, 0, deframer, 0);
        topology.connect(deframer, 0, p2s, 0);
        topology.connect(p2s, 0, collector, 0);
        topology.commit();
        POTHOS_TEST_TRUE(topology.waitInactive());
        //std::cout << topology.queryJSONStats() << std::endl;
    }

    std::cout << "verifyTestPlan" << std::endl;
    collector.callVoid("verifyTestPlan", expected);
}
