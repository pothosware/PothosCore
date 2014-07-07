// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Framework.hpp>
#include <Pothos/Proxy.hpp>
#include <Poco/TemporaryFile.h>
#include <iostream>

POTHOS_TEST_BLOCK("/blocks/tests", test_binary_file_blocks)
{
    auto env = Pothos::ProxyEnvironment::make("managed");
    auto registry = env->findProxy("Pothos/BlockRegistry");

    auto feeder = registry.callProxy("/blocks/sources/feeder_source", "int");
    auto collector = registry.callProxy("/blocks/sinks/collector_sink", "int");

    auto tempFile = Poco::TemporaryFile::tempName();
    auto fileSource = registry.callProxy("/blocks/binary_file_source");
    fileSource.callVoid("setFilePath", tempFile);

    auto fileSink = registry.callProxy("/blocks/binary_file_sink");
    fileSink.callVoid("setFilePath", tempFile);

    //feed buffer
    auto b0 = Pothos::BufferChunk(10*sizeof(int));
    auto p0 = b0.as<int *>();
    for (size_t i = 0; i < 10; i++) p0[i] = i;
    feeder.callProxy("feedBuffer", b0);

    //run a topology that sends feeder to file
    {
        Pothos::Topology topology;
        topology.connect(feeder, 0, fileSink, 0);
        topology.commit();
        POTHOS_TEST_TRUE(topology.waitInactive());
    }

    //run a topology that sends file to collector
    {
        Pothos::Topology topology;
        topology.connect(fileSource, 0, collector, 0);
        topology.commit();
        POTHOS_TEST_TRUE(topology.waitInactive());
    }

    //check the buffer for equality
    auto buff = collector.call<Pothos::BufferChunk>("getBuffer");
    POTHOS_TEST_EQUAL(buff.length, 10*sizeof(int));
    auto pb = buff.as<const int *>();
    for (int i = 0; i < 10; i++) POTHOS_TEST_EQUAL(pb[i], i);
}
