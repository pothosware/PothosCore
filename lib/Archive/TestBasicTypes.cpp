// Copyright (c) 2016-2016 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Object.hpp>
#include <Pothos/Testing.hpp>
#include <Pothos/Archive.hpp>
#include <sstream>
#include <iostream>
#include <cmath> //pow
#include <cstdlib> //rand

static const int numIters(100);

POTHOS_TEST_BLOCK("/archive/tests", test_integers)
{
    //test the boolean support
    for (int i = 0; i < 2; i++)
    {
        std::stringstream so;
        Pothos::Archive::OStreamArchiver ao(so);
        bool x((i==0)?false:true); ao << x;

        std::stringstream si(so.str());
        Pothos::Archive::IStreamArchiver ai(si);
        bool y; ai >> y;

        POTHOS_TEST_EQUAL(x, y);
    }

    //test the 32-bit integer support
    for (int i = 0; i < numIters; i++)
    {
        std::stringstream so;
        Pothos::Archive::OStreamArchiver ao(so);
        int x(std::rand()); ao << x;

        std::stringstream si(so.str());
        Pothos::Archive::IStreamArchiver ai(si);
        int y; ai >> y;

        POTHOS_TEST_EQUAL(x, y);
    }

    //test the 64-bit integer support
    for (int i = 0; i < numIters; i++)
    {
        std::stringstream so;
        Pothos::Archive::OStreamArchiver ao(so);
        long long x(std::rand());
        x <<= 32;
        x |= std::rand();
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
    for (int i = 0; i < numIters; i++)
    {
        std::stringstream so;
        Pothos::Archive::OStreamArchiver ao(so);
        float x(std::rand());
        x *= std::pow(1.0f, float(std::rand() & 0xf));
        ao << x;

        std::stringstream si(so.str());
        Pothos::Archive::IStreamArchiver ai(si);
        float y; ai >> y;

        POTHOS_TEST_EQUAL(x, y);
    }

    //test the double support
    for (int i = 0; i < numIters; i++)
    {
        std::stringstream so;
        Pothos::Archive::OStreamArchiver ao(so);
        double x(std::rand());
        x *= std::pow(1.0, double(std::rand() & 0xf));
        ao << x;

        std::stringstream si(so.str());
        Pothos::Archive::IStreamArchiver ai(si);
        double y; ai >> y;

        POTHOS_TEST_EQUAL(x, y);
    }
}

POTHOS_TEST_BLOCK("/archive/tests", test_complex)
{
    for (int i = 0; i < numIters; i++)
    {
        std::stringstream so;
        Pothos::Archive::OStreamArchiver ao(so);
        std::complex<int> x(std::rand(), std::rand());
        ao << x;

        std::stringstream si(so.str());
        Pothos::Archive::IStreamArchiver ai(si);
        std::complex<int> y; ai >> y;

        POTHOS_TEST_EQUAL(x, y);
    }
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
