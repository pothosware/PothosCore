// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Framework.hpp>
#include <Pothos/Proxy.hpp>
#include <Poco/Format.h>
#include <iostream>

static void network_test_harness(const std::string &scheme, const bool serverIsSource)
{
    std::cout << Poco::format("network_test_harness: %s:// (serverIsSource? %s)",
        scheme, std::string(serverIsSource?"true":"false")) << std::endl;
    auto env = Pothos::ProxyEnvironment::make("managed")->findProxy("Pothos/BlockRegistry");

    //create server
    auto server_uri = Poco::format("%s://0.0.0.0", scheme);
    std::cout << "make server " << server_uri << std::endl;
    auto server = env.callProxy(
        (serverIsSource)?"/blocks/network/network_source":"/blocks/network/network_sink",
        server_uri, "BIND", "int");

    //create client
    auto client_uri = Poco::format("%s://localhost:%s", scheme, server.call<std::string>("getActualPort"));
    std::cout << "make client " << client_uri << std::endl;
    auto client = env.callProxy(
        (serverIsSource)?"/blocks/network/network_sink":"/blocks/network/network_source",
        client_uri, "CONNECT", "int");

    //who is the source/sink?
    auto source = (serverIsSource)? server : client;
    auto sink = (serverIsSource)? client : server;

    //tester blocks
    auto feeder = env.callProxy("/blocks/sources/feeder_source", "int");
    auto collector = env.callProxy("/blocks/sinks/collector_sink", "int");

    //feed buffer
    auto b0 = Pothos::BufferChunk(10000*sizeof(int));
    auto p0 = b0.as<int *>();
    for (size_t i = 0; i < 10000; i++) p0[i] = i;
    feeder.callProxy("feedBuffer", b0);

    auto b1 = Pothos::BufferChunk(10000*sizeof(int));
    auto p1 = b1.as<int *>();
    for (size_t i = 0; i < 10000; i++) p1[i] = i+10000;
    feeder.callProxy("feedBuffer", b1);

    //feed labels within buffer length
    feeder.callProxy("feedLabel", Pothos::Label("lbl0", 3));
    feeder.callProxy("feedLabel", Pothos::Label("lbl1", 5));

    //feed some msgs
    feeder.callProxy("feedMessage", Pothos::Object("msg0"));
    feeder.callProxy("feedMessage", Pothos::Object("msg1"));
    feeder.callProxy("feedMessage", Pothos::Object(b0));

    //create tester topology
    std::cout << "Basic message test" << std::endl;
    {
        Pothos::Topology topology;
        topology.connect(source, 0, collector, 0);
        topology.connect(feeder, 0, sink, 0);
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
    POTHOS_TEST_EQUAL(msgs.size(), 3);
    POTHOS_TEST_TRUE(msgs[0].type() == typeid(std::string));
    POTHOS_TEST_TRUE(msgs[1].type() == typeid(std::string));
    POTHOS_TEST_TRUE(msgs[2].type() == typeid(Pothos::BufferChunk));
    POTHOS_TEST_EQUAL(msgs[0].extract<std::string>(), "msg0");
    POTHOS_TEST_EQUAL(msgs[1].extract<std::string>(), "msg1");

    //check the buffer for equality
    {
        auto mbuff = msgs[2].extract<Pothos::BufferChunk>();
        POTHOS_TEST_EQUAL(mbuff.length, 10000*sizeof(int));
        auto pb = mbuff.as<const int *>();
        for (int i = 0; i < 10000; i++) POTHOS_TEST_EQUAL(pb[i], i);
    }

    //check the buffer for equality
    POTHOS_TEST_EQUAL(buff.length, 2*10000*sizeof(int));
    auto pb = buff.as<const int *>();
    for (int i = 0; i < 2*10000; i++) POTHOS_TEST_EQUAL(pb[i], i);

    //check labels
    POTHOS_TEST_EQUAL(lbls.size(), 2);
    POTHOS_TEST_EQUAL(lbls[0].index, 3);
    POTHOS_TEST_EQUAL(lbls[1].index, 5);
    POTHOS_TEST_TRUE(lbls[0].data.type() == typeid(std::string));
    POTHOS_TEST_TRUE(lbls[1].data.type() == typeid(std::string));
    POTHOS_TEST_EQUAL(lbls[0].data.extract<std::string>(), "lbl0");
    POTHOS_TEST_EQUAL(lbls[1].data.extract<std::string>(), "lbl1");

    //create tester topology -- tests for open close
    std::cout << "Open/close repeat test" << std::endl;
    for (size_t i = 0; i < 10; i++)
    {
        Pothos::Topology topology;
        topology.connect(source, 0, collector, 0);
        topology.connect(feeder, 0, sink, 0);
        topology.commit();
    }

    std::cout << "Done!\n" << std::endl;
}

POTHOS_TEST_BLOCK("/blocks/tests", test_network_blocks)
{
    network_test_harness("tcp", true);
    network_test_harness("tcp", false);
    network_test_harness("udt", true);
    network_test_harness("udt", false);
}
