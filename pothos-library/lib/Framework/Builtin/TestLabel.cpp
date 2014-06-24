// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Framework.hpp>
#include <iostream>

POTHOS_TEST_BLOCK("/framework/tests", test_label_constructor)
{
    //test that perfect forwarding works

    auto label0 = Pothos::Label("hello", 0);
    POTHOS_TEST_TRUE(label0.data.type() == typeid(std::string));
    POTHOS_TEST_EQUAL(label0.data.extract<std::string>(), "hello");

    std::string value1("world");
    auto label1 = Pothos::Label(value1, 0);
    POTHOS_TEST_TRUE(label1.data.type() == typeid(std::string));
    POTHOS_TEST_EQUAL(label1.data.extract<std::string>(), value1);
}
