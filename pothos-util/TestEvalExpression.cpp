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

    //booleans
    const auto resultT = evalEnv.call<Pothos::Object>("eval", "true");
    POTHOS_TEST_TRUE(resultT.convert<bool>());

    const auto resultF = evalEnv.call<Pothos::Object>("eval", "false");
    POTHOS_TEST_TRUE(not resultF.convert<bool>());

    //simple expression
    const auto result = evalEnv.call<Pothos::Object>("eval", "1 + 2");
    POTHOS_TEST_EQUAL(result.convert<int>(), 3);

    //a pothos type
    const auto result2 = evalEnv.call<Pothos::Object>("eval", "DType(\"int32\")");
    POTHOS_TEST_TRUE(result2.convert<Pothos::DType>() == Pothos::DType(typeid(int)));

    //test string w/ escape quote
    const auto result3 = evalEnv.call<Pothos::Object>("eval", "\"hello \\\" world\"");
    POTHOS_TEST_EQUAL(result3.convert<std::string>(), "hello \" world");
}

POTHOS_TEST_BLOCK("/util/tests", test_eval_list_expression)
{
    auto env = Pothos::ProxyEnvironment::make("managed");
    auto evalEnv = env->findProxy("Pothos/Util/EvalEnvironment").callProxy("new");

    //the empty test
    {
        const auto result = evalEnv.call<Pothos::Object>("eval", "[]");
        const auto vec = result.convert<std::vector<int>>();
        POTHOS_TEST_EQUAL(vec.size(), 0);
    }

    //a simple test
    {
        const auto result = evalEnv.call<Pothos::Object>("eval", "[1, 2, 3]");
        const auto vec = result.convert<std::vector<int>>();
        POTHOS_TEST_EQUAL(vec.size(), 3);
        POTHOS_TEST_EQUAL(vec[0], 1);
        POTHOS_TEST_EQUAL(vec[1], 2);
        POTHOS_TEST_EQUAL(vec[2], 3);
    }

    //a trailing comma test
    {
        const auto result = evalEnv.call<Pothos::Object>("eval", "[1, ]");
        const auto vec = result.convert<std::vector<int>>();
        POTHOS_TEST_EQUAL(vec.size(), 1);
        POTHOS_TEST_EQUAL(vec[0], 1);
    }

    //a quote test (including commas and escapes)
    {
        const auto result = evalEnv.call<Pothos::Object>("eval", "[\"comma, \\\"comma, comma, \", \"chameleon\"]");
        const auto vec = result.convert<std::vector<std::string>>();
        POTHOS_TEST_EQUAL(vec.size(), 2);
        POTHOS_TEST_EQUAL(vec[0], "comma, \"comma, comma, ");
        POTHOS_TEST_EQUAL(vec[1], "chameleon");
    }

    //a nested test
    {
        const auto result = evalEnv.call<Pothos::Object>("eval", "[1, [\"hello\", \"world\"], 3]");
        const auto vec = result.convert<Pothos::ObjectVector>();
        POTHOS_TEST_EQUAL(vec.size(), 3);
        POTHOS_TEST_EQUAL(vec[0].convert<int>(), 1);
        const auto vec_1 = vec[1].convert<std::vector<std::string>>();
        POTHOS_TEST_EQUAL(vec_1.size(), 2);
        POTHOS_TEST_EQUAL(vec_1[0], "hello");
        POTHOS_TEST_EQUAL(vec_1[1], "world");
        POTHOS_TEST_EQUAL(vec[2].convert<int>(), 3);
    }
}

POTHOS_TEST_BLOCK("/util/tests", test_eval_map_expression)
{
    auto env = Pothos::ProxyEnvironment::make("managed");
    auto evalEnv = env->findProxy("Pothos/Util/EvalEnvironment").callProxy("new");

    //the empty test
    {
        const auto result = evalEnv.call<Pothos::Object>("eval", "{}");
        const auto map = result.convert<Pothos::ObjectMap>();
        POTHOS_TEST_EQUAL(map.size(), 0);
    }

    //a simple test
    {
        const auto result = evalEnv.call<Pothos::Object>("eval", "{\"hello\" : 1, \"world\" : 2}");
        const auto map = result.convert<Pothos::ObjectMap>();
        POTHOS_TEST_EQUAL(map.size(), 2);
        POTHOS_TEST_EQUAL(map.at(Pothos::Object("hello")).convert<int>(), 1);
        POTHOS_TEST_EQUAL(map.at(Pothos::Object("world")).convert<int>(), 2);
    }

    //a trailing comma test
    {
        const auto result = evalEnv.call<Pothos::Object>("eval", "{1:2, }");
        const auto map = result.convert<Pothos::ObjectMap>();
        POTHOS_TEST_EQUAL(map.size(), 1);
        POTHOS_TEST_EQUAL(map.at(Pothos::Object(1)).convert<int>(), 2);
    }

    //a nested test
    {
        const auto result = evalEnv.call<Pothos::Object>("eval", "{\"hello\" : 1, \"world\" : [1, 2, 3]}");
        const auto map = result.convert<Pothos::ObjectMap>();
        POTHOS_TEST_EQUAL(map.size(), 2);
        POTHOS_TEST_EQUAL(map.at(Pothos::Object("hello")).convert<int>(), 1);
        const auto vec_1 = map.at(Pothos::Object("world")).convert<std::vector<int>>();
        POTHOS_TEST_EQUAL(vec_1.size(), 3);
        POTHOS_TEST_EQUAL(vec_1[0], 1);
        POTHOS_TEST_EQUAL(vec_1[1], 2);
        POTHOS_TEST_EQUAL(vec_1[2], 3);
    }
}
