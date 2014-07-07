// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Framework.hpp>
#include <Pothos/Proxy.hpp>
#include <iostream>

POTHOS_TEST_BLOCK("/blocks/tests", test_signals_and_slots)
{
    auto env = Pothos::ProxyEnvironment::make("managed")->findProxy("Pothos/BlockRegistry");
    auto feeder = env.callProxy("/blocks/feeder_source", "int");
    auto collector = env.callProxy("/blocks/collector_sink", "int");
    auto messageToSignal = env.callProxy("/blocks/message_to_signal", "changeEvent");
    auto slotToMessage = env.callProxy("/blocks/slot_to_message", "handleEvent");

    //feed some msgs
    feeder.callProxy("feedMessage", Pothos::Object("msg0"));
    feeder.callProxy("feedMessage", Pothos::Object("msg1"));

    //run the topology
    {
        Pothos::Topology topology;
        topology.connect(feeder, 0, messageToSignal, 0);
        topology.connect(messageToSignal, "changeEvent", slotToMessage, "handleEvent");
        topology.connect(slotToMessage, 0, collector, 0);
        topology.commit();
        POTHOS_TEST_TRUE(topology.waitInactive());
    }

    //collect the messages
    auto msgs = collector.call<std::vector<Pothos::Object>>("getMessages");
    std::cout << msgs.size() << std::endl;

    //check msgs
    POTHOS_TEST_EQUAL(msgs.size(), 2);
    POTHOS_TEST_TRUE(msgs[0].type() == typeid(std::string));
    POTHOS_TEST_TRUE(msgs[1].type() == typeid(std::string));
    POTHOS_TEST_EQUAL(msgs[0].extract<std::string>(), "msg0");
    POTHOS_TEST_EQUAL(msgs[1].extract<std::string>(), "msg1");
}
