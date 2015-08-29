// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Framework.hpp>
#include <Pothos/Proxy.hpp>
#include <Pothos/Remote.hpp>
#include <Poco/JSON/Object.h>
#include <iostream>

static void delayBlockTestCase(const int delayVal)
{
    auto env = Pothos::ProxyEnvironment::make("managed");
    auto registry = env->findProxy("Pothos/BlockRegistry");
    auto feeder = registry.callProxy("/blocks/feeder_source", "int");
    auto delay = registry.callProxy("/comms/delay");
    auto collector = registry.callProxy("/blocks/collector_sink", "int");

    //setup
    std::cout << "delayBlockTestCase " << delayVal << std::endl;
    Pothos::BufferChunk buff0(typeid(int), 100);
    feeder.callVoid("feedBuffer", buff0);
    delay.callVoid("setDelay", delayVal);

    //run the topology
    std::cout << "run the topology\n";
    {
        Pothos::Topology topology;
        topology.connect(feeder, 0, delay, 0);
        topology.connect(delay, 0, collector, 0);
        topology.commit();
        POTHOS_TEST_TRUE(topology.waitInactive());
    }

    auto buff1 = collector.call<Pothos::BufferChunk>("getBuffer");
    POTHOS_TEST_EQUAL(buff1.elements(), size_t(100-delayVal));
}

POTHOS_TEST_BLOCK("/comms/tests", test_delay)
{
    delayBlockTestCase(0);
    delayBlockTestCase(10);
    delayBlockTestCase(-10);
}
