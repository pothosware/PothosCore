// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Framework.hpp>
#include <Pothos/Proxy.hpp>
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
;

POTHOS_TEST_BLOCK("/blocks/opencl/tests", test_opencl_kernel)
{
    auto registry = Pothos::ProxyEnvironment::make("managed")->findProxy("Pothos/BlockRegistry");
    auto collector = registry.callProxy("/blocks/sinks/collector_sink", "float32");

    auto feeder0 = registry.callProxy("/blocks/sources/feeder_source", "float32");
    auto feeder1 = registry.callProxy("/blocks/sources/feeder_source", "float32");

    auto openClKernel = registry.callProxy("/blocks/opencl/opencl_kernel", 0, 0);
    openClKernel.callVoid("setupInput", "0", "float32", "opencl"); //FIXME dont want to specify the domain here
    openClKernel.callVoid("setupInput", "1", "float32", "opencl"); //FIXME dont want to specify the domain here
    openClKernel.callVoid("setupOutput", "0", "float32", "opencl");
    openClKernel.callVoid("setSource", "add_2x_float32", KERNEL_SOURCE);
    openClKernel.callVoid("setLocalSize", 1);
    openClKernel.callVoid("setGlobalFactor", 1.0);
    openClKernel.callVoid("setProductionFactor", 1.0);

    //feed buffer
    auto b0 = Pothos::BufferChunk(10*sizeof(float));
    auto p0 = reinterpret_cast<float *>(b0.address);
    for (size_t i = 0; i < 10; i++) p0[i] = i;
    feeder0.callProxy("feedBuffer", b0);

    auto b1 = Pothos::BufferChunk(10*sizeof(float));
    auto p1 = reinterpret_cast<float *>(b1.address);
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
    auto pb = reinterpret_cast<float *>(buff.address);
    //for (int i = 0; i < 10; i++) std::cout << i << " " << pb[i] << std::endl;
    for (int i = 0; i < 10; i++) POTHOS_TEST_EQUAL(pb[i], float(i+i+10));
}
