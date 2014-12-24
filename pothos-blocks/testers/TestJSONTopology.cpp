// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Framework.hpp>
#include <Pothos/Proxy.hpp>
#include <Pothos/Remote.hpp>
#include <Poco/JSON/Object.h>
#include <iostream>

/*!
 * A hierarchy topology description in JSON.
 * Writing JSON in C++ quotes and escapes is not recomended.
 * This is just a unit test, consider passing a file path.
 */
static const char *PASSTHROUGH_JSON =
"{"
"   \"blocks\" : ["
"       {"
"           \"id\" : \"fwd\","
"           \"path\" : \"\\/blocks\\/gateway\","
"           \"calls\" : [[\"setMode\", \"\\\"FORWARD\\\"\"]]"
"       }"
"   ],"
"   \"connections\" : ["
"       [\"self\", \"in0\", \"fwd\", \"0\"],"
"       [\"fwd\", \"0\", \"self\", \"out0\"]"
"   ]"
"}"
;

POTHOS_TEST_BLOCK("/blocks/tests", test_json_topology)
{
    auto env = Pothos::ProxyEnvironment::make("managed");
    auto registry = env->findProxy("Pothos/BlockRegistry");
    auto feeder = registry.callProxy("/blocks/feeder_source", "int");
    auto collector = registry.callProxy("/blocks/collector_sink", "int");
    auto passthrough = Pothos::Topology::make(PASSTHROUGH_JSON);

    //create a test plan
    Poco::JSON::Object::Ptr testPlan(new Poco::JSON::Object());
    testPlan->set("enableBuffers", true);
    testPlan->set("enableLabels", true);
    testPlan->set("enableMessages", true);
    auto expected = feeder.callProxy("feedTestPlan", testPlan);

    //run the topology
    std::cout << "run the topology\n";
    {
        auto topology = env->findProxy("Pothos/Topology").callProxy("make");
        topology.callVoid("connect", feeder, "0", passthrough, "in0");
        topology.callVoid("connect", passthrough, "out0", collector, "0");
        topology.callVoid("commit");
        POTHOS_TEST_TRUE(topology.call<bool>("waitInactive"));
    }

    std::cout << "verifyTestPlan!\n";
    collector.callVoid("verifyTestPlan", expected);

    std::cout << "done!\n";
}
