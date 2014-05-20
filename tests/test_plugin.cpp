// Copyright (c) 2013-2013 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <boost/test/unit_test.hpp>
#include <stdexcept>
#include <iostream>

#include <Pothos/Plugin.hpp>

BOOST_AUTO_TEST_CASE(test_plugin_path)
{
    Pothos::PluginPath nullPath;
    BOOST_CHECK(nullPath == Pothos::PluginPath());
    BOOST_CHECK_EQUAL(nullPath.toString(), "/");
    BOOST_CHECK_EQUAL(nullPath.listNodes().size(), 0);

    Pothos::PluginPath goodPath("/foo-bar/my_module");
    BOOST_REQUIRE_EQUAL(goodPath.listNodes().size(), 2);
    BOOST_CHECK_EQUAL(goodPath.listNodes()[0], "foo-bar");
    BOOST_CHECK_EQUAL(goodPath.listNodes()[1], "my_module");

    BOOST_CHECK_THROW(Pothos::PluginPath("/foo-bar//my_module"), Pothos::PluginPathError);
    BOOST_CHECK_THROW(Pothos::PluginPath("/foo-bar/my_module "), Pothos::PluginPathError);
    BOOST_CHECK_THROW(Pothos::PluginPath("foo-bar/my_module"), Pothos::PluginPathError);
    BOOST_CHECK_THROW(Pothos::PluginPath("/foo bar/my_module"), Pothos::PluginPathError);
}

BOOST_AUTO_TEST_CASE(test_plugin_registry)
{
    Pothos::PluginRegistry::add(Pothos::Plugin("/tests/t0"));
    Pothos::PluginRegistry::add(Pothos::Plugin("/tests/t1"));
    BOOST_CHECK(Pothos::PluginRegistry::exists(Pothos::PluginPath("/tests/t0")));
    BOOST_CHECK(Pothos::PluginRegistry::exists(Pothos::PluginPath("/tests/t1")));
    BOOST_CHECK(not Pothos::PluginRegistry::exists(Pothos::PluginPath("/tests/t2")));

    std::vector<std::string> expectedList, testsList;
    expectedList.push_back("t0");
    expectedList.push_back("t1");
    testsList = Pothos::PluginRegistry::list(Pothos::PluginPath("/tests"));
    BOOST_CHECK_EQUAL_COLLECTIONS(testsList.begin(), testsList.end(), expectedList.begin(), expectedList.end());

    Pothos::PluginRegistry::remove(Pothos::PluginPath("/tests/t1"));
    BOOST_CHECK(not Pothos::PluginRegistry::exists(Pothos::PluginPath("/tests/t1")));
    expectedList.erase(std::find(expectedList.begin(), expectedList.end(), "t1"));
    testsList = Pothos::PluginRegistry::list(Pothos::PluginPath("/tests"));
    BOOST_CHECK_EQUAL_COLLECTIONS(testsList.begin(), testsList.end(), expectedList.begin(), expectedList.end());

    Pothos::PluginRegistry::add(Pothos::Plugin("/tests/foo/t2"));
    BOOST_CHECK(Pothos::PluginRegistry::exists(Pothos::PluginPath("/tests/foo")));
    BOOST_CHECK(Pothos::PluginRegistry::exists(Pothos::PluginPath("/tests/foo/t2")));
    expectedList.push_back("foo");
    testsList = Pothos::PluginRegistry::list(Pothos::PluginPath("/tests"));
    BOOST_CHECK_EQUAL_COLLECTIONS(testsList.begin(), testsList.end(), expectedList.begin(), expectedList.end());

    BOOST_CHECK_THROW(Pothos::PluginRegistry::add(Pothos::Plugin("/tests/t0")), Pothos::PluginRegistryError);
    BOOST_CHECK_THROW(Pothos::PluginRegistry::get(Pothos::PluginPath("/tests")), Pothos::PluginRegistryError);
    BOOST_CHECK_THROW(Pothos::PluginRegistry::remove(Pothos::PluginPath("/tests/foo")), Pothos::PluginRegistryError);
}
