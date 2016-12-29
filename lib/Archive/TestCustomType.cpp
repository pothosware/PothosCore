// Copyright (c) 2016-2016 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Object.hpp>
#include <Pothos/Testing.hpp>
#include <Pothos/Archive.hpp>
#include <sstream>
#include <iostream>

namespace PothosTesting {
struct CustomType0
{
    template <typename Archive>
    void serialize(Archive &a, const unsigned int)
    {
        a & fooInt;
        a & barStr;
        a & bazMap;
    }

    int fooInt;
    std::string barStr;
    std::map<int, std::string> bazMap;
};
} //namespace PothosTesting

POTHOS_TEST_BLOCK("/archive/tests", test_custom_type)
{
    PothosTesting::CustomType0 x;
    x.fooInt = 123;
    x.barStr = "hello";
    x.bazMap[42] = "42";

    std::stringstream so;
    Pothos::Archive::OStreamArchiver ao(so);
    ao << x;

    std::stringstream si(so.str());
    PothosTesting::CustomType0 y;
    Pothos::Archive::IStreamArchiver ai(si);
    ai >> y;

    POTHOS_TEST_EQUAL(x.fooInt, y.fooInt);
    POTHOS_TEST_EQUAL(x.barStr, y.barStr);
    POTHOS_TEST_EQUAL(x.bazMap, y.bazMap);
}
