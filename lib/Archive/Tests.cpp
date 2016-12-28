// Copyright (c) 2016-2016 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Object.hpp>
#include <Pothos/Testing.hpp>
#include <Pothos/Archive.hpp>
#include <sstream>
#include <iostream>

POTHOS_TEST_BLOCK("/archive/tests", test_integers)
{
    //test the 32-bit integer support
    {
        std::stringstream so;
        Pothos::Archive::OStreamArchiver ao(so);
        int x(123); ao << x;

        std::stringstream si(so.str());
        Pothos::Archive::IStreamArchiver ai(si);
        int y; ai >> y;

        POTHOS_TEST_EQUAL(x, y);
    }

    //test the 64-bit integer support
    {
        std::stringstream so;
        Pothos::Archive::OStreamArchiver ao(so);
        long long x(123);
        x <<= 32;
        ao << x;

        std::stringstream si(so.str());
        Pothos::Archive::IStreamArchiver ai(si);
        long long y; ai >> y;

        POTHOS_TEST_EQUAL(x, y);
    }
}

class BooHoo
{
public:
    BooHoo()
    {

    }

    int foo;
    std::string bar;
    std::map<int, std::string> baz;
};

namespace Pothos {
namespace serialization {

template<class Archive>
void serialize(Archive &a, BooHoo &t, const unsigned int)
{
    a & t.foo;
    a & t.bar;
    a & t.baz;
}

}}

POTHOS_CLASS_EXPORT_ID(BooHoo, "BooHoo");

POTHOS_TEST_BLOCK("/archive/tests", test_basic_archive)
{
    std::stringstream ss;
    BooHoo bh;
    bh.foo = 123;
    bh.bar = "hello";
    bh.baz[42] = "42";
    Pothos::Archive::OStreamArchiver ar(ss);
    ar << bh;
    //std::string s("xyz");
    //Pothos::Archive::serializeArchive(ss, s);

    std::stringstream ss2(ss.str());
    BooHoo bh2;
    Pothos::Archive::IStreamArchiver ar2(ss2);
    ar2 >> bh2;

    POTHOS_TEST_EQUAL(bh.foo, bh2.foo);
    POTHOS_TEST_EQUAL(bh.bar, bh2.bar);
    POTHOS_TEST_EQUAL(bh.baz, bh2.baz);
}
