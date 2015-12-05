// Copyright (c) 2015-2015 Rinat Zakirov
// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Framework.hpp>
#include <Pothos/Proxy.hpp>
#include <Pothos/Remote.hpp>
#include <Poco/JSON/Object.h>
#include <iostream>

POTHOS_TEST_BLOCK("/comms/tests", test_symbol_bit_conversions)
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

        //create the blocks
        auto feeder = registry.callProxy("/blocks/feeder_source", "uint8");
        auto symsToBits = registry.callProxy("/comms/symbols_to_bits");
        symsToBits.callProxy("setModulus", mod);
        symsToBits.callProxy("setBitOrder", order);
        auto bitsToSyms = registry.callProxy("/comms/bits_to_symbols");
        bitsToSyms.callProxy("setModulus", mod);
        bitsToSyms.callProxy("setBitOrder", order);
        auto collector = registry.callProxy("/blocks/collector_sink", "uint8");

        //setup the topology
        Pothos::Topology topology;
        topology.connect(feeder, 0, symsToBits, 0);
        topology.connect(symsToBits, 0, bitsToSyms, 0);
        topology.connect(bitsToSyms, 0, collector, 0);

        //create a test plan for streams
        std::cout << "Perform stream-based test plan..." << std::endl;
        Poco::JSON::Object::Ptr testPlan0(new Poco::JSON::Object());
        testPlan0->set("enableBuffers", true);
        testPlan0->set("enableLabels", true);
        testPlan0->set("minValue", 0);
        testPlan0->set("maxValue", (1 << mod) - 1);
        auto expected0 = feeder.callProxy("feedTestPlan", testPlan0);
        topology.commit();
        POTHOS_TEST_TRUE(topology.waitInactive(0.01));
        collector.callVoid("verifyTestPlan", expected0);

        //create a test plan for packets
        std::cout << "Perform packet-based test plan..." << std::endl;
        Poco::JSON::Object::Ptr testPlan1(new Poco::JSON::Object());
        testPlan1->set("enablePackets", true);
        testPlan1->set("enableLabels", true);
        testPlan1->set("minValue", 0);
        testPlan1->set("maxValue", (1 << mod) - 1);
        auto expected1 = feeder.callProxy("feedTestPlan", testPlan1);
        topology.commit();
        POTHOS_TEST_TRUE(topology.waitInactive(0.01));
        collector.callVoid("verifyTestPlan", expected1);
    }

    std::cout << "done!\n";
}
