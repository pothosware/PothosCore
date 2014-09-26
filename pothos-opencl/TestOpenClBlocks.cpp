// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Framework.hpp>
#include <Pothos/Proxy.hpp>
#include <Poco/JSON/Object.h>
#include <iostream>

static const char *KERNEL_SOURCE =
"__kernel void add_2x_float32(\n"
"    __global const float* in0,\n"
"    __global const float* in1,\n"
"    __global float* out\n"
")\n"
"{\n"
"    const uint i = get_global_id(0);\n"
"    out[i] = in0[i] + in1[i];\n"
"}\n"
"\n"
"__kernel void add_2x_complex64(\n"
"    __global const float2* in0,\n"
"    __global const float2* in1,\n"
"    __global float2* out\n"
")\n"
"{\n"
"    const uint i = get_global_id(0);\n"
"    out[i] = in0[i] + in1[i];\n"
"}"
"__kernel void copy_int(\n"
"    __global const int* in,\n"
"    __global int* out\n"
")\n"
"{\n"
"    const uint i = get_global_id(0);\n"
"    out[i] = in[i];\n"
"}"
;

POTHOS_TEST_BLOCK("/blocks/opencl/tests", test_opencl_kernel)
{
    auto registry = Pothos::ProxyEnvironment::make("managed")->findProxy("Pothos/BlockRegistry");
    auto collector = registry.callProxy("/blocks/collector_sink", "float32");

    auto feeder0 = registry.callProxy("/blocks/feeder_source", "float32");
    auto feeder1 = registry.callProxy("/blocks/feeder_source", "float32");

    auto openClKernel = registry.callProxy("/blocks/opencl_kernel", "0:0", std::vector<std::string>(2, "float"), std::vector<std::string>(1, "float"));
    openClKernel.callVoid("setSource", "add_2x_float32", KERNEL_SOURCE);
    openClKernel.callVoid("setLocalSize", 1);
    openClKernel.callVoid("setGlobalFactor", 1.0);
    openClKernel.callVoid("setProductionFactor", 1.0);

    //feed buffer
    auto b0 = Pothos::BufferChunk(10*sizeof(float));
    auto p0 = b0.as<float *>();
    for (size_t i = 0; i < 10; i++) p0[i] = i;
    feeder0.callProxy("feedBuffer", b0);

    auto b1 = Pothos::BufferChunk(10*sizeof(float));
    auto p1 = b1.as<float *>();
    for (size_t i = 0; i < 10; i++) p1[i] = i+10;
    feeder1.callProxy("feedBuffer", b1);

    //run the topology
    {
        Pothos::Topology topology;
        topology.connect(feeder0, 0, openClKernel, 0);
        topology.connect(feeder1, 0, openClKernel, 1);
        topology.connect(openClKernel, 0, collector, 0);
        topology.commit();
        POTHOS_TEST_TRUE(topology.waitInactive());
    }

    //get the buffer
    auto buff = collector.call<Pothos::BufferChunk>("getBuffer");
    std::cout << buff.length << std::endl;

    //check the buffer for equality
    POTHOS_TEST_EQUAL(buff.length, 10*sizeof(float));
    auto pb = buff.as<const float *>();
    //for (int i = 0; i < 10; i++) std::cout << i << " " << pb[i] << std::endl;
    for (int i = 0; i < 10; i++) POTHOS_TEST_EQUAL(pb[i], float(i+i+10));
}

POTHOS_TEST_BLOCK("/blocks/opencl/tests", test_opencl_kernel_back_to_back)
{
    auto registry = Pothos::ProxyEnvironment::make("managed")->findProxy("Pothos/BlockRegistry");
    auto collector = registry.callProxy("/blocks/collector_sink", "float32");

    auto feeder0 = registry.callProxy("/blocks/feeder_source", "float32");
    auto feeder1 = registry.callProxy("/blocks/feeder_source", "float32");
    auto feeder2 = registry.callProxy("/blocks/feeder_source", "float32");

    auto openClKernel0 = registry.callProxy("/blocks/opencl_kernel", "0:0", std::vector<std::string>(2, "float"), std::vector<std::string>(1, "float"));
    openClKernel0.callVoid("setSource", "add_2x_float32", KERNEL_SOURCE);
    openClKernel0.callVoid("setLocalSize", 1);
    openClKernel0.callVoid("setGlobalFactor", 1.0);
    openClKernel0.callVoid("setProductionFactor", 1.0);

    auto openClKernel1 = registry.callProxy("/blocks/opencl_kernel", "0:0", std::vector<std::string>(2, "float"), std::vector<std::string>(1, "float"));
    openClKernel1.callVoid("setSource", "add_2x_float32", KERNEL_SOURCE);
    openClKernel1.callVoid("setLocalSize", 1);
    openClKernel1.callVoid("setGlobalFactor", 1.0);
    openClKernel1.callVoid("setProductionFactor", 1.0);

    //feed buffer
    auto b0 = Pothos::BufferChunk(10*sizeof(float));
    auto p0 = b0.as<float *>();
    for (size_t i = 0; i < 10; i++) p0[i] = i;
    feeder0.callProxy("feedBuffer", b0);

    auto b1 = Pothos::BufferChunk(10*sizeof(float));
    auto p1 = b1.as<float *>();
    for (size_t i = 0; i < 10; i++) p1[i] = i+10;
    feeder1.callProxy("feedBuffer", b1);

    auto b2 = Pothos::BufferChunk(10*sizeof(float));
    auto p2 = b2.as<float *>();
    for (size_t i = 0; i < 10; i++) p2[i] = i+20;
    feeder2.callProxy("feedBuffer", b2);

    //run the topology
    {
        Pothos::Topology topology;
        topology.connect(feeder0, 0, openClKernel0, 0);
        topology.connect(feeder1, 0, openClKernel0, 1);
        topology.connect(openClKernel0, 0, openClKernel1, 0);
        topology.connect(feeder2, 0, openClKernel1, 1);
        topology.connect(openClKernel1, 0, collector, 0);
        topology.commit();
        std::cout << topology.toDotMarkup() << std::endl;
        POTHOS_TEST_TRUE(topology.waitInactive());
    }

    //get the buffer
    auto buff = collector.call<Pothos::BufferChunk>("getBuffer");
    std::cout << buff.length << std::endl;

    //check the buffer for equality
    POTHOS_TEST_EQUAL(buff.length, 10*sizeof(float));
    auto pb = buff.as<const float *>();
    //for (int i = 0; i < 10; i++) std::cout << i << " " << pb[i] << std::endl;
    for (int i = 0; i < 10; i++) POTHOS_TEST_EQUAL(pb[i], float(i+i+10+i+20));
}

POTHOS_TEST_BLOCK("/blocks/opencl/tests", test_opencl_kernel_middle_man)
{
    auto registry = Pothos::ProxyEnvironment::make("managed")->findProxy("Pothos/BlockRegistry");
    auto collector = registry.callProxy("/blocks/collector_sink", "int");
    auto collectorMiddle = registry.callProxy("/blocks/collector_sink", "int");
    auto feeder = registry.callProxy("/blocks/feeder_source", "int");

    auto openClKernel0 = registry.callProxy("/blocks/opencl_kernel", "0:0", std::vector<std::string>(1, "float"), std::vector<std::string>(1, "float"));
    openClKernel0.callVoid("setSource", "copy_int", KERNEL_SOURCE);
    openClKernel0.callVoid("setLocalSize", 1);
    openClKernel0.callVoid("setGlobalFactor", 1.0);
    openClKernel0.callVoid("setProductionFactor", 1.0);

    auto openClKernel1 = registry.callProxy("/blocks/opencl_kernel", "0:0", std::vector<std::string>(1, "float"), std::vector<std::string>(1, "float"));
    openClKernel1.callVoid("setSource", "copy_int", KERNEL_SOURCE);
    openClKernel1.callVoid("setLocalSize", 1);
    openClKernel1.callVoid("setGlobalFactor", 1.0);
    openClKernel1.callVoid("setProductionFactor", 1.0);

    //create test plan
    Poco::JSON::Object::Ptr testPlan(new Poco::JSON::Object());
    testPlan->set("enableBuffers", true);
    //large and numerous payloads
    testPlan->set("minTrials", 100);
    testPlan->set("maxTrials", 200);
    testPlan->set("minSize", 1024);
    testPlan->set("maxSize", 1024);
    auto expected = feeder.callProxy("feedTestPlan", testPlan);

    //create tester topology
    std::cout << "Make topology" << std::endl;
    {
        Pothos::Topology topology;
        topology.connect(feeder, 0, openClKernel0, 0);
        topology.connect(openClKernel0, 0, openClKernel1, 0);
        topology.connect(openClKernel0, 0, collectorMiddle, 0);
        topology.connect(openClKernel1, 0, collector, 0);
        topology.commit();
        POTHOS_TEST_TRUE(topology.waitInactive());
    }

    std::cout << "verifyTestPlan" << std::endl;
    collector.callVoid("verifyTestPlan", expected);
    collectorMiddle.callVoid("verifyTestPlan", expected);
}
