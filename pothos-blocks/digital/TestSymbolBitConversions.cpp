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

        auto feeder = registry.callProxy("/blocks/feeder_source", "uint8");
        auto collector = registry.callProxy("/blocks/collector_sink", "uint8");
        auto bytes2bits = registry.callProxy("/blocks/symbols_to_bits");
        auto bits2bytes = registry.callProxy("/blocks/bits_to_symbols");

        //create a test plan
        Poco::JSON::Object::Ptr testPlan(new Poco::JSON::Object());
        testPlan->set("enableBuffers", true);
        testPlan->set("minValue", 0);
        testPlan->set("maxValue", (1 << mod) - 1);

        bytes2bits.callProxy("setModulus", mod);
        bits2bytes.callProxy("setModulus", mod);
        bytes2bits.callProxy("setBitOrder", order);
        bits2bytes.callProxy("setBitOrder", order);

        Pothos::Topology topology;
        topology.connect(feeder, 0, bytes2bits, 0);
        topology.connect(bytes2bits, 0, bits2bytes, 0);
        topology.connect(bits2bytes, 0, collector, 0);
        topology.commit();

        auto expected = feeder.callProxy("feedTestPlan", testPlan);
        POTHOS_TEST_TRUE(topology.waitInactive());

        std::cout << "verifyTestPlan!\n";
        collector.callVoid("verifyTestPlan", expected);
    }

    std::cout << "done!\n";
}
