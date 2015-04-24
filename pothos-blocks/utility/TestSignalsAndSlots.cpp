// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Framework.hpp>
#include <Pothos/Proxy.hpp>
#include <iostream>

POTHOS_TEST_BLOCK("/blocks/tests", test_signals_and_slots)
{
    auto env = Pothos::ProxyEnvironment::make("managed");
    auto registry = env->findProxy("Pothos/BlockRegistry");
    auto feeder = registry.callProxy("/blocks/feeder_source", "int");
    auto collector = registry.callProxy("/blocks/collector_sink", "int");
    auto messageToSignal = registry.callProxy("/blocks/message_to_signal", "changeEvent");
    auto slotToMessage = registry.callProxy("/blocks/slot_to_message", "handleEvent");

    //feed some msgs
    feeder.callProxy("feedMessage", "msg0");
    feeder.callProxy("feedMessage", "msg1");

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

POTHOS_TEST_BLOCK("/blocks/tests", test_transform_signal)
{
    auto env = Pothos::ProxyEnvironment::make("managed");
    auto registry = env->findProxy("Pothos/BlockRegistry");

    auto feeder = registry.callProxy("/blocks/feeder_source", "int");
    auto collector = registry.callProxy("/blocks/collector_sink", "int");
    auto messageToSignal = registry.callProxy("/blocks/message_to_signal", "changeEvent");
    auto slotToMessage = registry.callProxy("/blocks/slot_to_message", "handleEvent");
    auto transform = registry.callProxy("/blocks/transform_signal", std::vector<std::string>(1, "val"));
    transform.callVoid("setExpression", "2*val");

    //feed some msgs
    feeder.callProxy("feedMessage", Pothos::Object(11));
    feeder.callProxy("feedMessage", Pothos::Object(-32));

    //run the topology
    {
        Pothos::Topology topology;
        topology.connect(feeder, 0, messageToSignal, 0);
        topology.connect(messageToSignal, "changeEvent", transform, "setVal");
        topology.connect(transform, "triggered", slotToMessage, "handleEvent");
        topology.connect(slotToMessage, 0, collector, 0);
        topology.commit();
        POTHOS_TEST_TRUE(topology.waitInactive());
    }

    //collect the messages
    auto msgs = collector.call<std::vector<Pothos::Object>>("getMessages");
    std::cout << msgs.size() << std::endl;

    //check msgs
    POTHOS_TEST_EQUAL(msgs.size(), 2);
    POTHOS_TEST_EQUAL(msgs[0].convert<int>(), 22);
    POTHOS_TEST_EQUAL(msgs[1].convert<int>(), -64);
}

POTHOS_TEST_BLOCK("/blocks/tests", test_transform_signal2)
{
    auto env = Pothos::ProxyEnvironment::make("managed");
    auto registry = env->findProxy("Pothos/BlockRegistry");

    auto feeder = registry.callProxy("/blocks/feeder_source", "int");
    auto collector = registry.callProxy("/blocks/collector_sink", "int");
    auto slotToMessage = registry.callProxy("/blocks/slot_to_message", "handleEvent");
    auto transform = registry.callProxy("/blocks/transform_signal", std::vector<std::string>(1, "val"));
    transform.callVoid("setExpression", "2*val0 + val1");

    //test message with two args - object vector format since we are not using messageToSignal
    std::vector<Pothos::Object> args;
    args.emplace_back(11);
    args.emplace_back(-32);
    feeder.callProxy("feedMessage", args);

    //run the topology
    {
        Pothos::Topology topology;
        topology.connect(feeder, 0, transform, "setVal");
        topology.connect(transform, "triggered", slotToMessage, "handleEvent");
        topology.connect(slotToMessage, 0, collector, 0);
        topology.commit();
        POTHOS_TEST_TRUE(topology.waitInactive());
    }

    //collect the messages
    auto msgs = collector.call<std::vector<Pothos::Object>>("getMessages");
    std::cout << msgs.size() << std::endl;

    //check msgs
    POTHOS_TEST_EQUAL(msgs.size(), 1);
    POTHOS_TEST_EQUAL(msgs[0].convert<int>(), 2*11 + -32);
}
