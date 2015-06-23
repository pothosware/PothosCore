// Copyright (c) 2015-2015 Rinat Zakirov
// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Framework.hpp>
#include <Pothos/Proxy.hpp>
#include <Pothos/Remote.hpp>
#include <Poco/JSON/Object.h>
#include <iostream>

POTHOS_TEST_BLOCK("/blocks/tests", test_symbol_bit_conversions)
{
    auto env = Pothos::ProxyEnvironment::make("managed");
    auto registry = env->findProxy("Pothos/BlockRegistry");

    //run the topology
    for (int mod = 1; mod <= 8; mod++)
    for (int i = 0; i < 2; i++)
    {
        const std::string order = i == 0 ? "LSBit" : "MSBit";
        std::cout << "run the topology with " << order << " order ";
        std::cout << "and " << mod << " modulus" << std::endl;

        //path 0 tests symbols to bits -> bits to symbols
        auto symsToBits0 = registry.callProxy("/blocks/symbols_to_bits");
        symsToBits0.callProxy("setModulus", mod);
        symsToBits0.callProxy("setBitOrder", order);
        auto bitsToSyms0 = registry.callProxy("/blocks/bits_to_symbols");
        bitsToSyms0.callProxy("setModulus", mod);
        bitsToSyms0.callProxy("setBitOrder", order);
        auto collector0 = registry.callProxy("/blocks/collector_sink", "uint8");

        //path 1 tests pkt in -> symbols to bits -> bits to symbols -> pkt out
        auto s2p1 = registry.callProxy("/blocks/stream_to_packet");
        auto symsToBits1 = registry.callProxy("/blocks/symbols_to_bits");
        symsToBits1.callProxy("setModulus", mod);
        symsToBits1.callProxy("setBitOrder", order);
        auto bitsToSyms1 = registry.callProxy("/blocks/bits_to_symbols");
        bitsToSyms1.callProxy("setModulus", mod);
        bitsToSyms1.callProxy("setBitOrder", order);
        auto p2s1 = registry.callProxy("/blocks/packet_to_stream");
        auto collector1 = registry.callProxy("/blocks/collector_sink", "uint8");

        //create a test plan
        auto feeder = registry.callProxy("/blocks/feeder_source", "uint8");
        Poco::JSON::Object::Ptr testPlan(new Poco::JSON::Object());
        testPlan->set("enableBuffers", true);
        testPlan->set("enableLabels", true);
        testPlan->set("minValue", 0);
        testPlan->set("maxValue", (1 << mod) - 1);
        auto expected = feeder.callProxy("feedTestPlan", testPlan);

        Pothos::Topology topology;

        //setup path 0
        topology.connect(feeder, 0, symsToBits0, 0);
        topology.connect(symsToBits0, 0, bitsToSyms0, 0);
        topology.connect(bitsToSyms0, 0, collector0, 0);

        //setup path 1
        topology.connect(feeder, 0, s2p1, 0);
        topology.connect(s2p1, 0, symsToBits1, 0);
        topology.connect(symsToBits1, 0, bitsToSyms1, 0);
        topology.connect(bitsToSyms1, 0, p2s1, 0);
        topology.connect(p2s1, 0, collector1, 0);

        topology.commit();
        POTHOS_TEST_TRUE(topology.waitInactive());

        std::cout << "verifyTestPlan0..." << std::endl;
        collector0.callVoid("verifyTestPlan", expected);
        std::cout << "verifyTestPlan1..." << std::endl;
        collector1.callVoid("verifyTestPlan", expected);
    }

    std::cout << "done!\n";
}
