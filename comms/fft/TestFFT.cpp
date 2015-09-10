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

    /*
    numpy.fft.fft([(0.4+0.6j), (-0.7+0.6j), (-0.2+0.8j), (0.9+0.2j)])
    array([  4.00000000e-01+2.2j,   1.00000000e+00+1.4j,  -5.55111512e-17+0.6j,
             2.00000000e-01-1.8j])
    */

    std::vector<std::complex<float>> input;
    input.emplace_back(0.4, 0.6);
    input.emplace_back(-0.7, 0.6);
    input.emplace_back(-0.2, 0.8);
    input.emplace_back(0.9, 0.2);

    std::vector<std::complex<float>> result;
    result.emplace_back(0.4, 2.2);
    result.emplace_back(1.0, 1.4);
    result.emplace_back(0.0, 0.6);
    result.emplace_back(0.2, -1.8);

    //create blocks
    const auto dtype = Pothos::DType(typeid(std::complex<float>));
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

    //check the buffer
    auto buff = collector.call<Pothos::BufferChunk>("getBuffer");
    POTHOS_TEST_EQUAL(buff.elements(), 4);
    auto pb = buff.as<const std::complex<float> *>();
    POTHOS_TEST_EQUAL(buff.elements(), 4);
    for (size_t i = 0; i < 4; i++)
    {
        std::cout << i << " expected " << result[i] << " actual " << pb[i] << std::endl;
        POTHOS_TEST_TRUE(std::abs(pb[i].real()-result[i].real()) < 0.01);
        POTHOS_TEST_TRUE(std::abs(pb[i].imag()-result[i].imag()) < 0.01);
    }
}

POTHOS_TEST_BLOCK("/comms/tests", test_fft_short)
{
    auto env = Pothos::ProxyEnvironment::make("managed");
    auto registry = env->findProxy("Pothos/BlockRegistry");

    /*
    numpy.fft.fft([(-4-6j), (-3+6j), 2j, (1-10j)])
    array([ -6. -8.j,  12. -4.j,  -2. +0.j, -20.-12.j])
    */

    std::vector<std::complex<float>> input;
    input.emplace_back(-4, -6);
    input.emplace_back(-3, 6);
    input.emplace_back(0, 2);
    input.emplace_back(1, -10);

    std::vector<std::complex<short>> result;
    result.emplace_back(-6, -8);
    result.emplace_back(12, -4);
    result.emplace_back(-2, 0);
    result.emplace_back(-20, -12);

    //create blocks
    const auto dtype = Pothos::DType(typeid(std::complex<short>));
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

    //check the buffer
    auto buff = collector.call<Pothos::BufferChunk>("getBuffer");
    POTHOS_TEST_EQUAL(buff.elements(), 4);
    auto pb = buff.as<const std::complex<short> *>();
    POTHOS_TEST_EQUAL(buff.elements(), 4);
    //POTHOS_TEST_EQUALA(pb, result.data(), 4);
    for (size_t i = 0; i < 4; i++)
    {
        std::cout << pb[i] << std::endl;
    }
}
