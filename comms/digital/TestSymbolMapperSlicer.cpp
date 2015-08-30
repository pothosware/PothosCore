// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Framework.hpp>
#include <Pothos/Proxy.hpp>
#include <Poco/JSON/Object.h>
#include <iostream>
#include <complex>

POTHOS_TEST_BLOCK("/comms/tests", test_symbol_mapper_slicer_float)
{
    auto env = Pothos::ProxyEnvironment::make("managed");
    auto registry = env->findProxy("Pothos/BlockRegistry");

    auto feeder = registry.callProxy("/blocks/feeder_source", "unsigned char");
    auto mapper = registry.callProxy("/comms/symbol_mapper", "float");
    auto slicer = registry.callProxy("/comms/symbol_slicer", "float");
    auto collector = registry.callProxy("/blocks/collector_sink", "unsigned char");

    static const float mapD[] = {-3, -1, 1, 3};
    std::vector<float> map(mapD, mapD+4);
    mapper.callProxy("setMap", map);
    slicer.callProxy("setMap", map);

    //load feeder blocks
    auto b0 = Pothos::BufferChunk(10*sizeof(unsigned char));
    auto p0 = b0.as<unsigned char *>();
    for (size_t i = 0; i < 10; i++) p0[i] = i&3;
    feeder.callProxy("feedBuffer", b0);

    //run the topology
    Pothos::Topology topology;
    topology.connect(feeder, 0, mapper, 0);
    topology.connect(mapper, 0, slicer, 0);
    topology.connect(slicer, 0, collector, 0);
    topology.commit();
    POTHOS_TEST_TRUE(topology.waitInactive());

    //check the collector
    auto buff = collector.call<Pothos::BufferChunk>("getBuffer");
    POTHOS_TEST_EQUAL(buff.length, 10*sizeof(unsigned char));
    auto pb = buff.as<const unsigned char *>();
    POTHOS_TEST_EQUALA(pb, p0, 10);

    //try random test plan
    collector.callVoid("clear");
    Poco::JSON::Object::Ptr testPlan(new Poco::JSON::Object());
    testPlan->set("enableBuffers", true);
    testPlan->set("minValue", 0);
    testPlan->set("maxValue", map.size()-1);
    auto expected = feeder.callProxy("feedTestPlan", testPlan);
    topology.commit();
    POTHOS_TEST_TRUE(topology.waitInactive());
    collector.callVoid("verifyTestPlan", expected);
}

POTHOS_TEST_BLOCK("/comms/tests", test_symbol_mapper_slicer_complex)
{
    auto registry = Pothos::ProxyEnvironment::make("managed")->findProxy("Pothos/BlockRegistry");

    auto feeder = registry.callProxy("/blocks/feeder_source", Pothos::DType(typeid(unsigned char)));
    auto mapper = registry.callProxy("/comms/symbol_mapper", Pothos::DType(typeid(std::complex<float>)));
    auto slicer = registry.callProxy("/comms/symbol_slicer", Pothos::DType(typeid(std::complex<float>)));
    auto collector = registry.callProxy("/blocks/collector_sink", Pothos::DType(typeid(unsigned char)));

    std::vector<std::complex<float>> map;
    map.emplace_back(-1.0, -1.0);
    map.emplace_back(-1.0, 1.0);
    map.emplace_back(1.0, -1.0);
    map.emplace_back(1.0, 1.0);
    mapper.callProxy("setMap", map);
    slicer.callProxy("setMap", map);

    //load feeder blocks
    auto b0 = Pothos::BufferChunk(10*sizeof(unsigned char));
    auto p0 = b0.as<unsigned char *>();
    for (size_t i = 0; i < 10; i++) p0[i] = i&3;
    feeder.callProxy("feedBuffer", b0);

    //run the topology
    Pothos::Topology topology;
    topology.connect(feeder, 0, mapper, 0);
    topology.connect(mapper, 0, slicer, 0);
    topology.connect(slicer, 0, collector, 0);
    topology.commit();
    POTHOS_TEST_TRUE(topology.waitInactive());

    //check the collector
    auto buff = collector.call<Pothos::BufferChunk>("getBuffer");
    POTHOS_TEST_EQUAL(buff.length, 10*sizeof(unsigned char));
    auto pb = buff.as<const unsigned char *>();
    POTHOS_TEST_EQUALA(pb, p0, 10);

    //try random test plan
    collector.callVoid("clear");
    Poco::JSON::Object::Ptr testPlan(new Poco::JSON::Object());
    testPlan->set("enableBuffers", true);
    testPlan->set("minValue", 0);
    testPlan->set("maxValue", map.size()-1);
    auto expected = feeder.callProxy("feedTestPlan", testPlan);
    topology.commit();
    POTHOS_TEST_TRUE(topology.waitInactive());
    collector.callVoid("verifyTestPlan", expected);
}

POTHOS_TEST_BLOCK("/comms/tests", test_symbol_mapper_gray_code)
{
    auto registry = Pothos::ProxyEnvironment::make("managed")->findProxy("Pothos/BlockRegistry");

    auto feeder = registry.callProxy("/blocks/feeder_source", Pothos::DType(typeid(unsigned char)));
    auto encode = registry.callProxy("/comms/symbol_mapper", Pothos::DType(typeid(int8_t)));
    auto decode = registry.callProxy("/comms/symbol_slicer", Pothos::DType(typeid(int8_t)));
    auto collector = registry.callProxy("/blocks/collector_sink", Pothos::DType(typeid(unsigned char)));

    std::vector<int> grayCode;
    grayCode.push_back(0);
    grayCode.push_back(1);
    grayCode.push_back(3);
    grayCode.push_back(2);
    grayCode.push_back(6);
    grayCode.push_back(7);
    grayCode.push_back(5);
    grayCode.push_back(4);
    encode.callVoid("setMap", grayCode);
    decode.callVoid("setMap", grayCode);

    //create test plan
    Poco::JSON::Object::Ptr testPlan(new Poco::JSON::Object());
    testPlan->set("enableBuffers", true);
    testPlan->set("minValue", 0);
    testPlan->set("maxValue", grayCode.size()-1);
    auto expected = feeder.callProxy("feedTestPlan", testPlan);

    //run the topology
    {
        Pothos::Topology topology;
        topology.connect(feeder, 0, encode, 0);
        topology.connect(encode, 0, decode, 0);
        topology.connect(decode, 0, collector, 0);
        topology.commit();
        POTHOS_TEST_TRUE(topology.waitInactive());
    }

    //verify the test plan
    collector.callVoid("verifyTestPlan", expected);
}

