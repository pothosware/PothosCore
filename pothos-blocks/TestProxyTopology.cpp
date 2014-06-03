// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Framework.hpp>
#include <Pothos/Proxy.hpp>
#include <iostream>

POTHOS_TEST_BLOCK("/blocks/tests", test_proxy_topology)
{
    auto env = Pothos::ProxyEnvironment::make("managed");
    auto registry = env->findProxy("Pothos/BlockRegistry");
    auto feeder = registry.callProxy("/blocks/sources/feeder_source", "int");
    auto collector = registry.callProxy("/blocks/sinks/collector_sink", "int");

    //feed some msgs
    std::cout << "give messages to the feeder\n";
    feeder.callProxy("feedMessage", Pothos::Object("msg0"));
    feeder.callProxy("feedMessage", Pothos::Object("msg1"));

    //run the topology
    std::cout << "run the topology\n";
    {
        auto topology = env->findProxy("Pothos/Topology").callProxy("new");
        topology.callProxy("connect", feeder, "0", collector, "0");
        topology.callProxy("commit");
        POTHOS_TEST_TRUE(topology.call<bool>("waitInactive"));
    }

    //check msgs
    std::cout << "check the collector for msgs\n";
    auto msgs = collector.call<std::vector<Pothos::Object>>("getMessages");
    POTHOS_TEST_EQUAL(msgs.size(), 2);
    POTHOS_TEST_EQUAL(msgs[0].extract<std::string>(), "msg0");
    POTHOS_TEST_EQUAL(msgs[1].extract<std::string>(), "msg1");

    std::cout << "done!\n";
}
