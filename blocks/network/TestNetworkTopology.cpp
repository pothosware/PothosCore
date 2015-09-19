// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Framework.hpp>
#include <Pothos/Proxy.hpp>
#include <Pothos/Remote.hpp>
#include <Poco/JSON/Object.h>
#include <iostream>

POTHOS_TEST_BLOCK("/blocks/tests", test_network_topology)
{
    //spawn a server and client
    std::cout << "create proxy server\n";
    Pothos::RemoteServer server("tcp://[::]");
    Pothos::RemoteClient client("tcp://[::1]:"+server.getActualPort());

    //local and remote block registries
    auto remoteReg = client.makeEnvironment("managed")->findProxy("Pothos/BlockRegistry");
    auto localReg = Pothos::ProxyEnvironment::make("managed")->findProxy("Pothos/BlockRegistry");

    //create local and remote unit test blocks
    std::cout << "create remote feeder\n";
    auto feeder = remoteReg.callProxy("/blocks/feeder_source", "int");
    std::cout << "create local collector\n";
    auto collector = localReg.callProxy("/blocks/collector_sink", "int");

    //create a test plan
    Poco::JSON::Object::Ptr testPlan(new Poco::JSON::Object());
    testPlan->set("enableBuffers", true);
    testPlan->set("enableLabels", true);
    testPlan->set("enableMessages", true);
    auto expected = feeder.callProxy("feedTestPlan", testPlan);

    //run the topology
    std::cout << "run the topology\n";
    {
        Pothos::Topology topology;
        topology.connect(feeder, 0, collector, 0);
        topology.commit();
        POTHOS_TEST_TRUE(topology.waitInactive());
    }

    collector.callVoid("verifyTestPlan", expected);
}
