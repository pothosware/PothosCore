// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Framework.hpp>
#include <Pothos/Proxy.hpp>
#include <Pothos/Remote.hpp>
#include <iostream>

POTHOS_TEST_BLOCK("/blocks/tests", test_network_topology)
{
    //spawn a server and client
    std::cout << "create proxy server\n";
    Pothos::RemoteServer server("tcp://0.0.0.0");
    Pothos::RemoteClient client("tcp://localhost:"+server.getActualPort());

    //local and remote block registries
    auto remoteReg = client.makeEnvironment("managed")->findProxy("Pothos/BlockRegistry");
    auto localReg = Pothos::ProxyEnvironment::make("managed")->findProxy("Pothos/BlockRegistry");

    //create local and remote unit test blocks
    std::cout << "create remote feeder\n";
    auto feeder = remoteReg.callProxy("/blocks/feeder_source", "int");
    std::cout << "create local collector\n";
    auto collector = localReg.callProxy("/blocks/collector_sink", "int");

    //feed some msgs
    std::cout << "give messages to the feeder\n";
    feeder.callProxy("feedMessage", Pothos::Object("msg0"));
    feeder.callProxy("feedMessage", Pothos::Object("msg1"));

    //run the topology
    std::cout << "run the topology\n";
    {
        Pothos::Topology topology;
        topology.connect(feeder, 0, collector, 0);
        topology.commit();
        POTHOS_TEST_TRUE(topology.waitInactive());
    }

    //check msgs
    std::cout << "check the collector for msgs\n";
    auto msgs = collector.call<std::vector<Pothos::Object>>("getMessages");
    POTHOS_TEST_EQUAL(msgs.size(), 2);
    POTHOS_TEST_EQUAL(msgs[0].extract<std::string>(), "msg0");
    POTHOS_TEST_EQUAL(msgs[1].extract<std::string>(), "msg1");

    std::cout << "done!\n";
}
