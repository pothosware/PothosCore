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

POTHOS_TEST_BLOCK("/archive/tests", test_floats)
{
    //test the float support
    {
        std::stringstream so;
        Pothos::Archive::OStreamArchiver ao(so);
        float x(1e6); ao << x;

        std::stringstream si(so.str());
        Pothos::Archive::IStreamArchiver ai(si);
        float y; ai >> y;

        POTHOS_TEST_EQUAL(x, y);
    }

    //test the double support
    {
        std::stringstream so;
        Pothos::Archive::OStreamArchiver ao(so);
        double x(-0.1e3); ao << x;

        std::stringstream si(so.str());
        Pothos::Archive::IStreamArchiver ai(si);
        double y; ai >> y;

        POTHOS_TEST_EQUAL(x, y);
    }
}

POTHOS_TEST_BLOCK("/archive/tests", test_complex)
{
    std::stringstream so;
    Pothos::Archive::OStreamArchiver ao(so);
    std::complex<int> x(1, -2); ao << x;

    std::stringstream si(so.str());
    Pothos::Archive::IStreamArchiver ai(si);
    std::complex<int> y; ai >> y;

    POTHOS_TEST_EQUAL(x, y);
}

POTHOS_TEST_BLOCK("/archive/tests", test_string)
{
    std::stringstream so;
    Pothos::Archive::OStreamArchiver ao(so);
    std::string x("hello world"); ao << x;

    std::stringstream si(so.str());
    Pothos::Archive::IStreamArchiver ai(si);
    std::string y; ai >> y;

    POTHOS_TEST_EQUAL(x, y);
}

POTHOS_TEST_BLOCK("/archive/tests", test_map)
{
    std::stringstream so;
    Pothos::Archive::OStreamArchiver ao(so);
    std::map<int, std::string> x;
    x[123] = "hello";
    x[456] = "world";
    ao << x;

    std::stringstream si(so.str());
    Pothos::Archive::IStreamArchiver ai(si);
    std::map<int, std::string> y; ai >> y;

    POTHOS_TEST_EQUAL(x, y);
}

POTHOS_TEST_BLOCK("/archive/tests", test_vector)
{
    std::stringstream so;
    Pothos::Archive::OStreamArchiver ao(so);
    std::vector<std::string> x;
    x.push_back("hello");
    x.push_back("world");
    ao << x;

    std::stringstream si(so.str());
    Pothos::Archive::IStreamArchiver ai(si);
    std::vector<std::string> y; ai >> y;

    POTHOS_TEST_EQUALV(x, y);
}

class BooHoo
{
public:
    BooHoo()
    {

    }

    template <typename Archive>
    void save(Archive &a, const unsigned int)
    {
        a & foo;
        a & bar;
        a & baz;
    }

    template <typename Archive>
    void load(Archive &a, const unsigned int)
    {
        a & foo;
        a & bar;
        a & baz;
    }

    POTHOS_SERIALIZATION_SPLIT_MEMBER()

    int foo;
    std::string bar;
    std::map<int, std::string> baz;
};


/*
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
*/

POTHOS_CLASS_EXPORT_GUID(BooHoo, "BooHoo");

POTHOS_TEST_BLOCK("/archive/tests", test_basic_archive)
{
    std::stringstream ss;
    BooHoo bh;
    bh.foo = 123;
    bh.bar = "hello";
    bh.baz[42] = "42";
    Pothos::Archive::OStreamArchiver ar(ss);
    ar << bh;

    std::stringstream ss2(ss.str());
    BooHoo bh2;
    Pothos::Archive::IStreamArchiver ar2(ss2);
    ar2 >> bh2;

    POTHOS_TEST_EQUAL(bh.foo, bh2.foo);
    POTHOS_TEST_EQUAL(bh.bar, bh2.bar);
    POTHOS_TEST_EQUAL(bh.baz, bh2.baz);
}
