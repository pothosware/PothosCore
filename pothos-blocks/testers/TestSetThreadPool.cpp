// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Framework.hpp>
#include <Pothos/Proxy.hpp>
#include <Poco/JSON/Object.h>
#include <iostream>

POTHOS_TEST_BLOCK("/blocks/tests", test_set_thread_pool)
{
    auto env = Pothos::ProxyEnvironment::make("managed");
    auto registry = env->findProxy("Pothos/BlockRegistry");
    auto feeder = registry.callProxy("/blocks/feeder_source", "int");
    auto collector = registry.callProxy("/blocks/collector_sink", "int");

    //set one pool here
    Pothos::ThreadPool tp0(1);
    feeder.callVoid("setThreadPool", tp0);

    //create a test plan
    Poco::JSON::Object::Ptr testPlan(new Poco::JSON::Object());
    testPlan->set("enableBuffers", true);
    testPlan->set("enableLabels", true);
    testPlan->set("enableMessages", true);
    auto expected = feeder.callProxy("feedTestPlan", testPlan);

    //run the topology
    {
        Pothos::Topology topology;
        topology.connect(feeder, 0, collector, 0);
        //and set another after connecting
        Pothos::ThreadPool tp1(1);
        collector.callVoid("setThreadPool", tp1);
        topology.commit();
        POTHOS_TEST_TRUE(topology.waitInactive());
    }

    collector.callVoid("verifyTestPlan", expected);
}
