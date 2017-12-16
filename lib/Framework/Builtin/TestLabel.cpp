// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Framework.hpp>
#include <iostream>

POTHOS_TEST_BLOCK("/framework/tests", test_label_constructor)
{
    //test that perfect forwarding works

    auto label0 = Pothos::Label("id0", "test0", 0);
    POTHOS_TEST_EQUAL(label0.id, "id0");
    POTHOS_TEST_TRUE(label0.data.type() == typeid(std::string));
    POTHOS_TEST_EQUAL(label0.data.extract<std::string>(), "test0");

    std::string value1("test1");
    auto label1 = Pothos::Label("id1", value1, 0);
    POTHOS_TEST_TRUE(label1.data.type() == typeid(std::string));
    POTHOS_TEST_EQUAL(label1.data.extract<std::string>(), value1);

    auto label2 = Pothos::Label("id2", Pothos::Object("test2"), 0);
    POTHOS_TEST_TRUE(label2.data.type() == typeid(std::string));
    POTHOS_TEST_EQUAL(label2.data.extract<std::string>(), "test2");

    auto label3 = Pothos::Label("id3", "test3", 0);
    POTHOS_TEST_TRUE(label3.data.type() == typeid(std::string));
    POTHOS_TEST_EQUAL(label3.data.extract<std::string>(), "test3");
}
