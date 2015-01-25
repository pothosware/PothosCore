// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Framework.hpp>
#include <Pothos/Proxy.hpp>
#include <iostream>
#include <complex>

POTHOS_TEST_BLOCK("/blocks/tests", test_bytes_to_symbols_msb)
{
    auto registry = Pothos::ProxyEnvironment::make("managed")->findProxy("Pothos/BlockRegistry");

    auto feeder0 = registry.callProxy("/blocks/feeder_source", "unsigned char");
    auto b2s = registry.callProxy("/blocks/bytes_to_symbols");
    auto collector = registry.callProxy("/blocks/collector_sink", "unsigned char");

    const size_t modulus = 3;
    b2s.callProxy("setModulus", modulus);
    b2s.callProxy("setBitOrder", "MSBit");

    const size_t buflen = 3;

    //load feeder blocks
    auto b0 = Pothos::BufferChunk(buflen*sizeof(unsigned char));
    auto p0 = b0.as<unsigned char *>();
    p0[0] = 0xa3;
    p0[1] = 0x77;
    p0[2] = 0x15;
    feeder0.callProxy("feedBuffer", b0);

    //run the topology
    {
        Pothos::Topology topology;
        topology.connect(feeder0, 0, b2s, 0);
        topology.connect(b2s, 0, collector, 0);
        topology.commit();
        POTHOS_TEST_TRUE(topology.waitInactive());
    }

    //check the collector
    auto buff = collector.call<Pothos::BufferChunk>("getBuffer");
    POTHOS_TEST_EQUAL(buff.length, buflen*sizeof(unsigned char)*8/modulus);
    auto pb = buff.as<const unsigned char *>();
    //for (int i = 0; i < buff.length; i++) std::cout << i << " " << int(pb[i]) << std::endl;

    //[a37715] unpacked is [101 000 110 111 011 100 010 101]
    POTHOS_TEST_EQUAL(pb[0], 5);
    POTHOS_TEST_EQUAL(pb[1], 0);
    POTHOS_TEST_EQUAL(pb[2], 6);
    POTHOS_TEST_EQUAL(pb[3], 7);
    POTHOS_TEST_EQUAL(pb[4], 3);
    POTHOS_TEST_EQUAL(pb[5], 4);
    POTHOS_TEST_EQUAL(pb[6], 2);
    POTHOS_TEST_EQUAL(pb[7], 5);
}

POTHOS_TEST_BLOCK("/blocks/tests", test_bytes_to_symbols_lsb)
{
    auto registry = Pothos::ProxyEnvironment::make("managed")->findProxy("Pothos/BlockRegistry");

    auto feeder0 = registry.callProxy("/blocks/feeder_source", "unsigned char");
    auto b2s = registry.callProxy("/blocks/bytes_to_symbols");
    auto collector = registry.callProxy("/blocks/collector_sink", "unsigned char");

    const size_t modulus = 3;
    b2s.callProxy("setModulus", modulus);
    b2s.callProxy("setBitOrder", "LSBit");

    const size_t buflen = 3;

    //load feeder blocks
    auto b0 = Pothos::BufferChunk(buflen*sizeof(unsigned char));
    auto p0 = b0.as<unsigned char *>();
    p0[0] = 0xa3;
    p0[1] = 0x77;
    p0[2] = 0x15;
    feeder0.callProxy("feedBuffer", b0);

    //run the topology
    {
        Pothos::Topology topology;
        topology.connect(feeder0, 0, b2s, 0);
        topology.connect(b2s, 0, collector, 0);
        topology.commit();
        POTHOS_TEST_TRUE(topology.waitInactive());
    }

    //check the collector
    auto buff = collector.call<Pothos::BufferChunk>("getBuffer");
    POTHOS_TEST_EQUAL(buff.length, buflen*sizeof(unsigned char)*8/modulus);
    auto pb = buff.as<const unsigned char *>();
    //for (int i = 0; i < buff.length; i++) std::cout << i << " " << int(pb[i]) << std::endl;

    //[a37715] unpacked is [10100011 01110111 00010101]
    //in LSBit order that's...
    //[110 001 011 110 111 010 101 000]
    POTHOS_TEST_EQUAL(pb[0], 6);
    POTHOS_TEST_EQUAL(pb[1], 1);
    POTHOS_TEST_EQUAL(pb[2], 3);
    POTHOS_TEST_EQUAL(pb[3], 6);
    POTHOS_TEST_EQUAL(pb[4], 7);
    POTHOS_TEST_EQUAL(pb[5], 2);
    POTHOS_TEST_EQUAL(pb[6], 5);
    POTHOS_TEST_EQUAL(pb[7], 0);
}
