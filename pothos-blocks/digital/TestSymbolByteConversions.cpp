// Copyright (c) 2015-2015 Rinat Zakirov
// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Framework.hpp>
#include <Pothos/Proxy.hpp>
#include <Pothos/Remote.hpp>
#include <Poco/JSON/Object.h>
#include <iostream>

POTHOS_TEST_BLOCK("/blocks/tests", test_symbol_byte_conversions)
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

        //path 0 tests symbols to bytes -> bytes to symbols
        auto symsToBytes0 = registry.callProxy("/blocks/symbols_to_bytes");
        symsToBytes0.callProxy("setModulus", mod);
        symsToBytes0.callProxy("setBitOrder", order);
        auto bytesToSyms0 = registry.callProxy("/blocks/bytes_to_symbols");
        bytesToSyms0.callProxy("setModulus", mod);
        bytesToSyms0.callProxy("setBitOrder", order);
        auto collector0 = registry.callProxy("/blocks/collector_sink", "uint8");

        //path 1 tests symbols to bytes -> symbols to bits (8) -> bits to symbols
        auto symsToBytes1 = registry.callProxy("/blocks/symbols_to_bytes");
        symsToBytes1.callProxy("setModulus", mod);
        symsToBytes1.callProxy("setBitOrder", order);
        auto symsToBits1 = registry.callProxy("/blocks/symbols_to_bits");
        symsToBits1.callProxy("setModulus", 8);
        symsToBits1.callProxy("setBitOrder", order);
        auto bitsToSyms1 = registry.callProxy("/blocks/bits_to_symbols");
        bitsToSyms1.callProxy("setModulus", mod);
        bitsToSyms1.callProxy("setBitOrder", order);
        auto collector1 = registry.callProxy("/blocks/collector_sink", "uint8");

        //path 2 tests symbols to bits -> bits to symbols (8) -> bytes to symbols
        auto symsToBits2 = registry.callProxy("/blocks/symbols_to_bits");
        symsToBits2.callProxy("setModulus", mod);
        symsToBits2.callProxy("setBitOrder", order);
        auto bitsToSyms2 = registry.callProxy("/blocks/bits_to_symbols");
        bitsToSyms2.callProxy("setModulus", 8);
        bitsToSyms2.callProxy("setBitOrder", order);
        auto bytesToSyms2 = registry.callProxy("/blocks/bytes_to_symbols");
        bytesToSyms2.callProxy("setModulus", mod);
        bytesToSyms2.callProxy("setBitOrder", order);
        auto collector2 = registry.callProxy("/blocks/collector_sink", "uint8");

        //create a test plan
        //total multiple required to flush out complete stream
        auto feeder = registry.callProxy("/blocks/feeder_source", "uint8");
        Poco::JSON::Object::Ptr testPlan(new Poco::JSON::Object());
        testPlan->set("enableBuffers", true);
        testPlan->set("totalMultiple", 8);
        testPlan->set("minValue", 0);
        testPlan->set("maxValue", (1 << mod) - 1);
        auto expected = feeder.callProxy("feedTestPlan", testPlan);

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

        topology.commit();
        POTHOS_TEST_TRUE(topology.waitInactive());

        std::cout << "verifyTestPlan0..." << std::endl;
        collector0.callVoid("verifyTestPlan", expected);
        std::cout << "verifyTestPlan1..." << std::endl;
        collector1.callVoid("verifyTestPlan", expected);
        std::cout << "verifyTestPlan2..." << std::endl;
        collector2.callVoid("verifyTestPlan", expected);
    }

    std::cout << "done!\n";
}

POTHOS_TEST_BLOCK("/blocks/tests", test_symbol_byte_conversions_pkt)
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

        //path 3 tests pkt in -> symbols to bytes -> bytes to symbols -> pkt out
        auto s2p3 = registry.callProxy("/blocks/stream_to_packet");
        auto symsToBytes3 = registry.callProxy("/blocks/symbols_to_bytes");
        symsToBytes3.callProxy("setModulus", mod);
        symsToBytes3.callProxy("setBitOrder", order);
        auto bytesToSyms3 = registry.callProxy("/blocks/bytes_to_symbols");
        bytesToSyms3.callProxy("setModulus", mod);
        bytesToSyms3.callProxy("setBitOrder", order);
        auto p2s3 = registry.callProxy("/blocks/packet_to_stream");
        auto collector3 = registry.callProxy("/blocks/collector_sink", "uint8");

        //create a test plan
        //buffer multiple required to avoid padding packets in loopback test
        auto feeder = registry.callProxy("/blocks/feeder_source", "uint8");
        Poco::JSON::Object::Ptr testPlan(new Poco::JSON::Object());
        testPlan->set("enableBuffers", true);
        testPlan->set("bufferMultiple", 8);
        testPlan->set("minValue", 0);
        testPlan->set("maxValue", (1 << mod) - 1);
        auto expected = feeder.callProxy("feedTestPlan", testPlan);

        Pothos::Topology topology;

        //setup path 3
        topology.connect(feeder, 0, s2p3, 0);
        topology.connect(s2p3, 0, symsToBytes3, 0);
        topology.connect(symsToBytes3, 0, bytesToSyms3, 0);
        topology.connect(bytesToSyms3, 0, p2s3, 0);
        topology.connect(p2s3, 0, collector3, 0);

        topology.commit();
        POTHOS_TEST_TRUE(topology.waitInactive());

        std::cout << "verifyTestPlan3..." << std::endl;
        collector3.callVoid("verifyTestPlan", expected);
    }

    std::cout << "done!\n";
}
