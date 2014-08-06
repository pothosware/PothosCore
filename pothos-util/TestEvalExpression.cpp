// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Proxy.hpp>
#include <Pothos/Framework.hpp>
#include <Pothos/Exception.hpp>
#include <Pothos/Object/Containers.hpp>
#include <iostream>

POTHOS_TEST_BLOCK("/util/tests", test_eval_expression)
{
    //check that the following does not throw
    auto env = Pothos::ProxyEnvironment::make("managed");
    auto EvalEnvironment = env->findProxy("Pothos/Util/EvalEnvironment");
    auto evalEnv = EvalEnvironment.callProxy("new");

    const auto result = evalEnv.call<Pothos::Object>("eval", "1 + 2");
    POTHOS_TEST_EQUAL(result.convert<int>(), 3);

    const auto result2 = evalEnv.call<Pothos::Object>("eval", "DType(\"int32\")");
    POTHOS_TEST_TRUE(result2.convert<Pothos::DType>() == Pothos::DType(typeid(int)));
}


POTHOS_TEST_BLOCK("/util/tests", test_eval_list_expression)
{
    auto env = Pothos::ProxyEnvironment::make("managed");
    auto evalEnv = env->findProxy("Pothos/Util/EvalEnvironment").callProxy("new");

    const auto result = evalEnv.call<Pothos::Object>("eval", "[1, 2, 3]");
    const auto vec = result.convert<Pothos::ObjectVector>();
    POTHOS_TEST_EQUAL(vec.size(), 3);
    POTHOS_TEST_EQUAL(vec[0].convert<int>(), 1);
    POTHOS_TEST_EQUAL(vec[1].convert<int>(), 2);
    POTHOS_TEST_EQUAL(vec[2].convert<int>(), 3);
}

POTHOS_TEST_BLOCK("/util/tests", test_eval_map_expression)
{
    auto env = Pothos::ProxyEnvironment::make("managed");
    auto evalEnv = env->findProxy("Pothos/Util/EvalEnvironment").callProxy("new");

    const auto result = evalEnv.call<Pothos::Object>("eval", "{\"hello\" : 1, \"world\" : 2}");
    const auto map = result.convert<Pothos::ObjectMap>();
    POTHOS_TEST_EQUAL(map.size(), 2);
    POTHOS_TEST_EQUAL(map.at(Pothos::Object("hello")).convert<int>(), 1);
    POTHOS_TEST_EQUAL(map.at(Pothos::Object("world")).convert<int>(), 2);
}
