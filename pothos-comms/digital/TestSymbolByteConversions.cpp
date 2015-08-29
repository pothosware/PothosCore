// Copyright (c) 2015-2015 Rinat Zakirov
// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Framework.hpp>
#include <Pothos/Proxy.hpp>
#include <Pothos/Remote.hpp>
#include <Poco/JSON/Object.h>
#include <iostream>

POTHOS_TEST_BLOCK("/comms/tests", test_symbol_byte_conversions)
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

        auto feeder = registry.callProxy("/blocks/feeder_source", "uint8");

        //path 0 tests symbols to bytes -> bytes to symbols
        auto symsToBytes0 = registry.callProxy("/comms/symbols_to_bytes");
        symsToBytes0.callProxy("setModulus", mod);
        symsToBytes0.callProxy("setBitOrder", order);
        auto bytesToSyms0 = registry.callProxy("/comms/bytes_to_symbols");
        bytesToSyms0.callProxy("setModulus", mod);
        bytesToSyms0.callProxy("setBitOrder", order);
        auto collector0 = registry.callProxy("/blocks/collector_sink", "uint8");

        //path 1 tests symbols to bytes -> symbols to bits (8) -> bits to symbols
        auto symsToBytes1 = registry.callProxy("/comms/symbols_to_bytes");
        symsToBytes1.callProxy("setModulus", mod);
        symsToBytes1.callProxy("setBitOrder", order);
        auto symsToBits1 = registry.callProxy("/comms/symbols_to_bits");
        symsToBits1.callProxy("setModulus", 8);
        symsToBits1.callProxy("setBitOrder", order);
        auto bitsToSyms1 = registry.callProxy("/comms/bits_to_symbols");
        bitsToSyms1.callProxy("setModulus", mod);
        bitsToSyms1.callProxy("setBitOrder", order);
        auto collector1 = registry.callProxy("/blocks/collector_sink", "uint8");

        //path 2 tests symbols to bits -> bits to symbols (8) -> bytes to symbols
        auto symsToBits2 = registry.callProxy("/comms/symbols_to_bits");
        symsToBits2.callProxy("setModulus", mod);
        symsToBits2.callProxy("setBitOrder", order);
        auto bitsToSyms2 = registry.callProxy("/comms/bits_to_symbols");
        bitsToSyms2.callProxy("setModulus", 8);
        bitsToSyms2.callProxy("setBitOrder", order);
        auto bytesToSyms2 = registry.callProxy("/comms/bytes_to_symbols");
        bytesToSyms2.callProxy("setModulus", mod);
        bytesToSyms2.callProxy("setBitOrder", order);
        auto collector2 = registry.callProxy("/blocks/collector_sink", "uint8");

        Pothos::Topology topology;

        //setup path 0
        topology.connect(feeder, 0, symsToBytes0, 0);
        topology.connect(symsToBytes0, 0, bytesToSyms0, 0);
        topology.connect(bytesToSyms0, 0, collector0, 0);

        //setup path 1
        topology.connect(feeder, 0, symsToBytes1, 0);
        topology.connect(symsToBytes1, 0, symsToBits1, 0);
        topology.connect(symsToBits1, 0, bitsToSyms1, 0);
        topology.connect(bitsToSyms1, 0, collector1, 0);

        //setup path 2
        topology.connect(feeder, 0, symsToBits2, 0);
        topology.connect(symsToBits2, 0, bitsToSyms2, 0);
        topology.connect(bitsToSyms2, 0, bytesToSyms2, 0);
        topology.connect(bytesToSyms2, 0, collector2, 0);

        //create a test plan for streams
        //total multiple required to flush out complete stream
        std::cout << "Perform stream-based test plan..." << std::endl;
        {
            Poco::JSON::Object::Ptr testPlan(new Poco::JSON::Object());
            testPlan->set("enableBuffers", true);
            testPlan->set("totalMultiple", 8);
            testPlan->set("minValue", 0);
            testPlan->set("maxValue", (1 << mod) - 1);
            auto expected = feeder.callProxy("feedTestPlan", testPlan);
            topology.commit();
            POTHOS_TEST_TRUE(topology.waitInactive());

            std::cout << "verifyTestPlan path0..." << std::endl;
            collector0.callVoid("verifyTestPlan", expected);
            std::cout << "verifyTestPlan path1..." << std::endl;
            collector1.callVoid("verifyTestPlan", expected);
            std::cout << "verifyTestPlan path2..." << std::endl;
            collector2.callVoid("verifyTestPlan", expected);
        }

        //create a test plan for packets
        //buffer multiple required to avoid padding packets in loopback test
        std::cout << "Perform packet-based test plan..." << std::endl;
        {
            Poco::JSON::Object::Ptr testPlan(new Poco::JSON::Object());
            testPlan->set("enablePackets", true);
            testPlan->set("bufferMultiple", 8);
            testPlan->set("minValue", 0);
            testPlan->set("maxValue", (1 << mod) - 1);
            auto expected = feeder.callProxy("feedTestPlan", testPlan);
            topology.commit();
            POTHOS_TEST_TRUE(topology.waitInactive());

            std::cout << "verifyTestPlan path0..." << std::endl;
            collector0.callVoid("verifyTestPlan", expected);
            std::cout << "verifyTestPlan path1..." << std::endl;
            collector1.callVoid("verifyTestPlan", expected);
            std::cout << "verifyTestPlan path2..." << std::endl;
            collector2.callVoid("verifyTestPlan", expected);
        }
    }

    std::cout << "done!\n";
}
