// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Framework.hpp>
#include <Pothos/Proxy.hpp>
#include <Poco/TemporaryFile.h>
#include <Poco/JSON/Object.h>
#include <iostream>

POTHOS_TEST_BLOCK("/blocks/tests", test_binary_file_blocks)
{
    auto env = Pothos::ProxyEnvironment::make("managed");
    auto registry = env->findProxy("Pothos/BlockRegistry");

    auto feeder = registry.callProxy("/blocks/feeder_source", "int");
    auto collector = registry.callProxy("/blocks/collector_sink", "int");

    auto tempFile = Poco::TemporaryFile();
    std::cout << "tempFile " << tempFile.path() << std::endl;
    POTHOS_TEST_TRUE(tempFile.createFile());

    auto fileSource = registry.callProxy("/blocks/binary_file_source", "int");
    fileSource.callVoid("setFilePath", tempFile.path());

    auto fileSink = registry.callProxy("/blocks/binary_file_sink");
    fileSink.callVoid("setFilePath", tempFile.path());

    //create a test plan
    Poco::JSON::Object::Ptr testPlan(new Poco::JSON::Object());
    testPlan->set("enableBuffers", true);
    testPlan->set("minTrials", 100);
    testPlan->set("maxTrials", 200);
    testPlan->set("minSize", 512);
    testPlan->set("maxSize", 2048);
    auto expected = feeder.callProxy("feedTestPlan", testPlan);

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

    collector.callVoid("verifyTestPlan", expected);
}
