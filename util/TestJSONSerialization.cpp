// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Object.hpp>
#include <Pothos/Testing.hpp>
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Object.h>
#include <iostream>
#include <sstream>

POTHOS_TEST_BLOCK("/util/tests", test_json_serialization)
{
    Poco::JSON::Array::Ptr a0 = new Poco::JSON::Array();
    a0->add(123);
    a0->add(false);
    a0->add("word");
    POTHOS_TEST_TRUE(a0);
    Pothos::Object o0(a0);
    POTHOS_TEST_TRUE(o0);

    std::stringstream ss;
    o0.serialize(ss);
    std::cout << ss.str() << std::endl;

    Pothos::Object o1;
    o1.deserialize(ss);

    POTHOS_TEST_TRUE(o1);
    auto a1 = o1.extract<Poco::JSON::Array::Ptr>();
    POTHOS_TEST_TRUE(a1);

    POTHOS_TEST_EQUAL(a1->size(), 3);
    POTHOS_TEST_EQUAL(a1->getElement<int>(0), 123);
    POTHOS_TEST_EQUAL(a1->getElement<bool>(1), false);
    POTHOS_TEST_EQUAL(a1->getElement<std::string>(2), "word");
}

POTHOS_TEST_BLOCK("/util/tests", test_json_serialization_null)
{
    Poco::JSON::Array::Ptr a0;
    POTHOS_TEST_TRUE(not a0);
    Pothos::Object o0(a0);
    POTHOS_TEST_TRUE(o0);

    std::stringstream ss;
    o0.serialize(ss);
    std::cout << ss.str() << std::endl;

    Pothos::Object o1;
    o1.deserialize(ss);

    POTHOS_TEST_TRUE(o1);
    auto a1 = o1.extract<Poco::JSON::Array::Ptr>();
    POTHOS_TEST_TRUE(not a1);
}
