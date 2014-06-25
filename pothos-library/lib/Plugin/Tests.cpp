// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Plugin.hpp>
#include <Pothos/Testing.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>

POTHOS_TEST_BLOCK("/plugin/tests", test_plugin_path)
{
    Pothos::PluginPath nullPath;
    POTHOS_TEST_TRUE(nullPath == Pothos::PluginPath());
    POTHOS_TEST_EQUAL(nullPath.toString(), "/");
    POTHOS_TEST_EQUAL(nullPath.listNodes().size(), 0);

    Pothos::PluginPath goodPath("/foo-bar/my_module");
    POTHOS_TEST_EQUAL(goodPath.listNodes().size(), 2);
    POTHOS_TEST_EQUAL(goodPath.listNodes()[0], "foo-bar");
    POTHOS_TEST_EQUAL(goodPath.listNodes()[1], "my_module");

    POTHOS_TEST_THROWS(Pothos::PluginPath("/foo-bar//my_module"), Pothos::PluginPathError);
    POTHOS_TEST_THROWS(Pothos::PluginPath("/foo-bar/my_module "), Pothos::PluginPathError);
    POTHOS_TEST_THROWS(Pothos::PluginPath("foo-bar/my_module"), Pothos::PluginPathError);
    POTHOS_TEST_THROWS(Pothos::PluginPath("/foo bar/my_module"), Pothos::PluginPathError);
}

POTHOS_TEST_BLOCK("/plugin/tests", test_plugin_registry)
{
    Pothos::PluginRegistry::add(Pothos::Plugin("/tests/t0"));
    Pothos::PluginRegistry::add(Pothos::Plugin("/tests/t1"));
    POTHOS_TEST_TRUE(Pothos::PluginRegistry::exists(Pothos::PluginPath("/tests/t0")));
    POTHOS_TEST_TRUE(Pothos::PluginRegistry::exists(Pothos::PluginPath("/tests/t1")));
    POTHOS_TEST_TRUE(not Pothos::PluginRegistry::exists(Pothos::PluginPath("/tests/t2")));

    std::vector<std::string> expectedList, testsList;
    expectedList.push_back("t0");
    expectedList.push_back("t1");
    testsList = Pothos::PluginRegistry::list(Pothos::PluginPath("/tests"));
    POTHOS_TEST_EQUALV(testsList, expectedList);

    Pothos::PluginRegistry::remove(Pothos::PluginPath("/tests/t1"));
    POTHOS_TEST_TRUE(not Pothos::PluginRegistry::exists(Pothos::PluginPath("/tests/t1")));
    expectedList.erase(std::find(expectedList.begin(), expectedList.end(), "t1"));
    testsList = Pothos::PluginRegistry::list(Pothos::PluginPath("/tests"));
    POTHOS_TEST_EQUALV(testsList, expectedList);

    Pothos::PluginRegistry::add(Pothos::Plugin("/tests/foo/t2"));
    POTHOS_TEST_TRUE(Pothos::PluginRegistry::exists(Pothos::PluginPath("/tests/foo")));
    POTHOS_TEST_TRUE(Pothos::PluginRegistry::exists(Pothos::PluginPath("/tests/foo/t2")));
    expectedList.push_back("foo");
    testsList = Pothos::PluginRegistry::list(Pothos::PluginPath("/tests"));
    POTHOS_TEST_EQUALV(testsList, expectedList);

    POTHOS_TEST_THROWS(Pothos::PluginRegistry::add(Pothos::Plugin("/tests/t0")), Pothos::PluginRegistryError);
    POTHOS_TEST_THROWS(Pothos::PluginRegistry::get(Pothos::PluginPath("/tests")), Pothos::PluginRegistryError);
    POTHOS_TEST_THROWS(Pothos::PluginRegistry::remove(Pothos::PluginPath("/tests/foo")), Pothos::PluginRegistryError);
}
