// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <Pothos/Managed.hpp>
#include <Pothos/Testing.hpp>
#include <Pothos/Proxy.hpp>
#include <Poco/JSON/Object.h>
#include <iostream>

POTHOS_TEST_BLOCK("/proxy/python/tests", python_module_import)
{
    auto env = Pothos::ProxyEnvironment::make("python");
    env->findProxy("Pothos");
}

POTHOS_TEST_BLOCK("/proxy/python/tests", test_python_module)
{
    auto env = Pothos::ProxyEnvironment::make("python");
    env->findProxy("TestPothos").callProxy("main");
}

POTHOS_TEST_BLOCK("/proxy/python/tests", test_python_block)
{
    auto reg = Pothos::ProxyEnvironment::make("managed")->findProxy("Pothos/BlockRegistry");
    auto feeder = reg.callProxy("/blocks/feeder_source", "int");
    auto collector = reg.callProxy("/blocks/collector_sink", "int");
    auto forwarder = reg.callProxy("/python/forwarder", Pothos::DType("int"));

    //create a test plan
    Poco::JSON::Object::Ptr testPlan(new Poco::JSON::Object());
    testPlan->set("enableBuffers", true);
    testPlan->set("enableLabels", true);
    testPlan->set("enableMessages", true);
    auto expected = feeder.callProxy("feedTestPlan", testPlan);

    //run the topology
    {
        Pothos::Topology topology;
        topology.connect(feeder, 0, forwarder, 0);
        topology.connect(forwarder, 0, collector, 0);
        std::cout << "topology commit\n";
        topology.commit();
        POTHOS_TEST_TRUE(topology.waitInactive());
    }

    collector.callVoid("verifyTestPlan", expected);
    std::cout << "run done\n";
}

POTHOS_TEST_BLOCK("/proxy/python/tests", test_signals_and_slots)
{
    auto env = Pothos::ProxyEnvironment::make("python");
    auto emitter = env->findProxy("TestBlocks").callProxy("SimpleSignalEmitter");
    auto handler = env->findProxy("TestBlocks").callProxy("SimpleSlotHandler");

    //run the topology
    {
        Pothos::Topology topology;
        topology.connect(emitter, "activateCalled", handler, "activateHandler");
        std::cout << "topology commit\n";
        topology.commit();
        POTHOS_TEST_TRUE(topology.waitInactive());
    }

    auto lastWord = handler.call<std::string>("getLastWord");
    POTHOS_TEST_EQUAL(lastWord, "hello");
}
