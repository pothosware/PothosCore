// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <Pothos/Managed.hpp>
#include <Pothos/Testing.hpp>
#include <Pothos/Proxy.hpp>
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
    auto env = Pothos::ProxyEnvironment::make("python");
    auto forwarder = env->findProxy("TestBlocks").callProxy("Forwarder", Pothos::DType("int"));

    auto reg = Pothos::ProxyEnvironment::make("managed")->findProxy("Pothos/BlockRegistry");
    auto feeder = reg.callProxy("/blocks/sources/feeder_source", "int");
    auto collector = reg.callProxy("/blocks/sources/collector_sink", "int");

    //feed some msgs
    feeder.callProxy("feedMessage", Pothos::Object("msg0"));
    feeder.callProxy("feedMessage", Pothos::Object("msg1"));

    //feed buffer
    auto b0 = Pothos::BufferChunk(10*sizeof(int));
    int *p0 = reinterpret_cast<int *>(b0.address);
    for (size_t i = 0; i < 10; i++) p0[i] = i;
    feeder.callProxy("feedBuffer", b0);

    auto b1 = Pothos::BufferChunk(10*sizeof(int));
    int *p1 = reinterpret_cast<int *>(b1.address);
    for (size_t i = 0; i < 10; i++) p1[i] = i+10;
    feeder.callProxy("feedBuffer", b1);

    //feed labels within buffer length
    feeder.callProxy("feedLabel", Pothos::Label("lbl0", 3));
    feeder.callProxy("feedLabel", Pothos::Label("lbl1", 5));

    //run the topology
    {
        Pothos::Topology topology;
        topology.connect(feeder, 0, forwarder, 0);
        topology.connect(forwarder, 0, collector, 0);
        std::cout << "topology commit\n";
        topology.commit();
        POTHOS_TEST_TRUE(topology.waitInactive());
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
    int *pb = reinterpret_cast<int *>(buff.address);
    for (int i = 0; i < 20; i++) POTHOS_TEST_EQUAL(pb[i], i);

    //check labels
    POTHOS_TEST_EQUAL(lbls.size(), 2);
    POTHOS_TEST_EQUAL(lbls[0].index, 3);
    POTHOS_TEST_EQUAL(lbls[1].index, 5);
    POTHOS_TEST_TRUE(lbls[0].data.type() == typeid(std::string));
    POTHOS_TEST_TRUE(lbls[1].data.type() == typeid(std::string));
    POTHOS_TEST_EQUAL(lbls[0].data.extract<std::string>(), "lbl0");
    POTHOS_TEST_EQUAL(lbls[1].data.extract<std::string>(), "lbl1");

    std::cout << "run done\n";
}
