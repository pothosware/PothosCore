// Copyright (c) 2014-2017 Josh Blum
//                    2020 Nicholas Corgan
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Proxy.hpp>
#include <Pothos/Proxy/Environment.hpp>
#include <Pothos/Exception.hpp>
#include <iostream>
#include <json.hpp>

using json = nlohmann::json;

POTHOS_TEST_BLOCK("/util/tests", test_doc_utils_dump_json)
{
    //check that the following does not throw
    auto env = Pothos::ProxyEnvironment::make("managed");
    auto proxy = env->findProxy("Pothos/Util/DocUtils");
    const std::string jsonStr = proxy.call("dumpJson");
    POTHOS_TEST_FALSE(jsonStr.empty());
    if (jsonStr.size() > 100)
    {
        std::cout << jsonStr.substr(0, 100) << "...\n..." << jsonStr.substr(jsonStr.size()-100) << std::endl;
    }
    json::parse(jsonStr); //should not throw
}
