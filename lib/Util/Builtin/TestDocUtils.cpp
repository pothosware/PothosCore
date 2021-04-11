// Copyright (c) 2014-2017 Josh Blum
//               2020-2021 Nicholas Corgan
// SPDX-License-Identifier: BSL-1.0

#include "Testing/ScopedPlugin.hpp"

#include <Pothos/Testing.hpp>
#include <Pothos/Proxy.hpp>
#include <Pothos/Proxy/Environment.hpp>
#include <Pothos/Exception.hpp>
#include <iostream>
#include <json.hpp>

using json = nlohmann::json;

POTHOS_TEST_BLOCK("/util/tests", test_doc_utils_dump_json)
{
    auto env = Pothos::ProxyEnvironment::make("managed");
    const std::string docPath = "/blocks/docs/tests/doc_utils_dump_json/test_path";
    auto pluginRegistryProxy = env->findProxy("Pothos/PluginRegistry");

    {
        // A block needs to be in the registry for the output of
        // dumpJson to not be empty. It's unlikely this will be
        // called without any blocks, but just to be safe...
        auto plugin = ScopedPlugin<std::string>(docPath, std::string("{}"));
        POTHOS_TEST_TRUE(pluginRegistryProxy.call<bool>("exists", docPath));

        //check that the following does not throw
        auto docUtilsProxy = env->findProxy("Pothos/Util/DocUtils");
        const std::string jsonStr = docUtilsProxy.call("dumpJson");
        POTHOS_TEST_FALSE(jsonStr.empty());
        POTHOS_TEST_FALSE(json::parse(jsonStr).empty());
        if (jsonStr.size() > 100)
        {
            std::cout << jsonStr.substr(0, 100) << "...\n..." << jsonStr.substr(jsonStr.size()-100) << std::endl;
        }
    }

    POTHOS_TEST_FALSE(pluginRegistryProxy.call<bool>("exists", docPath));
}
