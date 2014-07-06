// Copyright (c) 2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Proxy.hpp>
#include <Pothos/Proxy/Environment.hpp>
#include <Pothos/Exception.hpp>
#include <Poco/JSON/Array.h>
#include <iostream>

POTHOS_TEST_BLOCK("/gui/tests", test_doc_utils_dump_json)
{
    //check that the following does not throw
    auto env = Pothos::ProxyEnvironment::make("managed");
    auto proxy = env->findProxy("Pothos/Gui/DocUtils");
    const auto json = proxy.call<Poco::JSON::Array::Ptr>("dumpJson");
    POTHOS_TEST_TRUE(json);
}
