// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Framework.hpp>
#include <Pothos/Proxy.hpp>
#include <iostream>
#include <vector>
#include <complex>

POTHOS_TEST_BLOCK("/comms/tests", test_fft_float)
{
    auto env = Pothos::ProxyEnvironment::make("managed");
    auto registry = env->findProxy("Pothos/BlockRegistry");

    const auto dtype = Pothos::DType(typeid(std::complex<float>));

    std::vector<std::complex<float>> input;
    input.emplace_back(1, 2);
    input.emplace_back(0, -1);
    input.emplace_back(0.5, 0);
    input.emplace_back(-0.5, 1);

    std::vector<std::complex<float>> result;
    result.emplace_back(1, 2);
    result.emplace_back(-1.5, 1.5);
    result.emplace_back(2, 2);
    result.emplace_back(2.5, 2.5);

    auto source = registry.callProxy("/blocks/vector_source", dtype);
    source.callVoid("setElements", input);
    source.callVoid("setMode", "ONCE");
    auto collector = registry.callProxy("/blocks/collector_sink", dtype);
    auto fft = registry.callProxy("/comms/fft", dtype, 4, false);

    //run the topology
    Pothos::Topology topology;
    topology.connect(source, 0, fft, 0);
    topology.connect(fft, 0, collector, 0);
    topology.commit();
    POTHOS_TEST_TRUE(topology.waitInactive());

    auto buff = collector.call<Pothos::BufferChunk>("getBuffer");
    POTHOS_TEST_EQUAL(buff.elements(), 4);
    auto pb = buff.as<const std::complex<float> *>();
    POTHOS_TEST_EQUAL(buff.elements(), 4);
    POTHOS_TEST_EQUALA(pb, result.data(), 4);
}
