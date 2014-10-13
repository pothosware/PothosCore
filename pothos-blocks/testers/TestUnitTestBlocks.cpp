// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Framework.hpp>
#include <Pothos/Proxy.hpp>
#include <Poco/JSON/Object.h>
#include <iostream>

POTHOS_TEST_BLOCK("/blocks/tests", test_unit_test_blocks)
{
    auto env = Pothos::ProxyEnvironment::make("managed");
    auto registry = env->findProxy("Pothos/BlockRegistry");
    auto feeder = registry.callProxy("/blocks/feeder_source", "int");
    auto collector = registry.callProxy("/blocks/collector_sink", "int");

    //feed some msgs
    feeder.callProxy("feedMessage", Pothos::Object("msg0"));
    feeder.callProxy("feedMessage", Pothos::Object("msg1"));

    //feed buffer
    auto b0 = Pothos::BufferChunk(10*sizeof(int));
    auto p0 = b0.as<int *>();
    for (size_t i = 0; i < 10; i++) p0[i] = i;
    feeder.callProxy("feedBuffer", b0);

    auto b1 = Pothos::BufferChunk(10*sizeof(int));
    auto p1 = b1.as<int *>();
    for (size_t i = 0; i < 10; i++) p1[i] = i+10;
    feeder.callProxy("feedBuffer", b1);

    //feed labels within buffer length
    feeder.callProxy("feedLabel", Pothos::Label("id0", "lbl0", 3));
    feeder.callProxy("feedLabel", Pothos::Label("id1", "lbl1", 5));

    //run the topology
    {
        Pothos::Topology topology;
        topology.connect(feeder, 0, collector, 0);
        topology.commit();
        POTHOS_TEST_TRUE(topology.waitInactive());
        std::cout << topology.toDotMarkup() << std::endl;
    }

    //collect the output
    auto msgs = collector.call<std::vector<Pothos::Object>>("getMessages");
    auto lbls = collector.call<std::vector<Pothos::Label>>("getLabels");
    auto buff = collector.call<Pothos::BufferChunk>("getBuffer");
    std::cout << msgs.size() << std::endl;
    std::cout << lbls.size() << std::endl;
    std::cout << buff.length << std::endl;

    //check msgs
    POTHOS_TEST_EQUAL(msgs.size(), 2);
    POTHOS_TEST_TRUE(msgs[0].type() == typeid(std::string));
    POTHOS_TEST_TRUE(msgs[1].type() == typeid(std::string));
    POTHOS_TEST_EQUAL(msgs[0].extract<std::string>(), "msg0");
    POTHOS_TEST_EQUAL(msgs[1].extract<std::string>(), "msg1");

    //check the buffer for equality
    POTHOS_TEST_EQUAL(buff.length, 20*sizeof(int));
    auto pb = buff.as<const int *>();
    for (int i = 0; i < 20; i++) POTHOS_TEST_EQUAL(pb[i], i);

    //check labels
    POTHOS_TEST_EQUAL(lbls.size(), 2);
    POTHOS_TEST_EQUAL(lbls[0].id, "id0");
    POTHOS_TEST_EQUAL(lbls[1].id, "id1");
    POTHOS_TEST_EQUAL(lbls[0].index, 3);
    POTHOS_TEST_EQUAL(lbls[1].index, 5);
    POTHOS_TEST_TRUE(lbls[0].data.type() == typeid(std::string));
    POTHOS_TEST_TRUE(lbls[1].data.type() == typeid(std::string));
    POTHOS_TEST_EQUAL(lbls[0].data.extract<std::string>(), "lbl0");
    POTHOS_TEST_EQUAL(lbls[1].data.extract<std::string>(), "lbl1");
}

POTHOS_TEST_BLOCK("/blocks/tests", test_random_unit_test)
{
    auto env = Pothos::ProxyEnvironment::make("managed");
    auto registry = env->findProxy("Pothos/BlockRegistry");
    auto feeder = registry.callProxy("/blocks/feeder_source", "int");
    auto collector = registry.callProxy("/blocks/collector_sink", "int");

    //create a test plan
    Poco::JSON::Object::Ptr testPlan(new Poco::JSON::Object());
    testPlan->set("enableBuffers", true);
    testPlan->set("enableLabels", true);
    testPlan->set("enableMessages", true);
    auto expected = feeder.callProxy("feedTestPlan", testPlan);

    //std::stringstream ss;
    //expected.convert<Poco::JSON::Object::Ptr>()->stringify(ss, 4);
    //std::cout << ss.str() << std::endl;

    //run the topology
    {
        Pothos::Topology topology;
        topology.connect(feeder, 0, collector, 0);
        topology.commit();
        POTHOS_TEST_TRUE(topology.waitInactive());
    }

    collector.callVoid("verifyTestPlan", expected);
}
