// Copyright (c) 2014-2017 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Proxy.hpp>
#include <Pothos/Proxy/Environment.hpp>
#include <Pothos/Exception.hpp>
#include <Poco/JSON/Parser.h>
#include <iostream>

POTHOS_TEST_BLOCK("/util/tests", test_doc_utils_dump_json)
{
    //check that the following does not throw
    auto env = Pothos::ProxyEnvironment::make("managed");
    auto proxy = env->findProxy("Pothos/Util/DocUtils");
    const auto json = proxy.call<std::string>("dumpJson");
    POTHOS_TEST_TRUE(not json.empty());
    if (json.size() > 100)
    {
        std::cout << json.substr(0, 100) << "...\n..." << json.substr(json.size()-100) << std::endl;
    }
    Poco::JSON::Parser().parse(json); //should not throw
}
