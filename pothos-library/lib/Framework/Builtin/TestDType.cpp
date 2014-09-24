// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Object.hpp>
#include <Pothos/Framework/DType.hpp>
#include <Pothos/Framework/Exception.hpp>
#include <complex>
#include <iostream>

POTHOS_TEST_BLOCK("/framework/tests", test_dtype_throws)
{
    POTHOS_TEST_THROWS(Pothos::DType("FooBar"), Pothos::DTypeUnknownError);
    POTHOS_TEST_THROWS(Pothos::DType("int 20"), Pothos::DTypeUnknownError);
    POTHOS_TEST_THROWS(Pothos::DType(typeid(std::string)), Pothos::DTypeUnknownError);
}

POTHOS_TEST_BLOCK("/framework/tests", test_dtype_equality)
{
    POTHOS_TEST_TRUE(Pothos::DType() == Pothos::DType());

    POTHOS_TEST_TRUE(Pothos::DType("int") == Pothos::DType(typeid(int)));
    POTHOS_TEST_TRUE(not (Pothos::DType("int") == Pothos::DType(typeid(unsigned int))));
    POTHOS_TEST_TRUE(Pothos::DType("uint") == Pothos::DType(typeid(unsigned int)));
    POTHOS_TEST_TRUE(Pothos::DType("float64") == Pothos::DType(typeid(double)));
    POTHOS_TEST_TRUE(Pothos::DType("complex128") == Pothos::DType(typeid(std::complex<double>)));
}

POTHOS_TEST_BLOCK("/framework/tests", test_dtype_sizes)
{
    POTHOS_TEST_EQUAL(Pothos::DType().size(), 0);
    POTHOS_TEST_EQUAL(Pothos::DType("").size(), 0);
    POTHOS_TEST_EQUAL(Pothos::DType("none").size(), 0);
    POTHOS_TEST_EQUAL(Pothos::DType("custom").size(), 1);

    POTHOS_TEST_EQUAL(Pothos::DType("byte").size(), sizeof(char));
    POTHOS_TEST_EQUAL(Pothos::DType("char").size(), sizeof(char));
    POTHOS_TEST_EQUAL(Pothos::DType("short").size(), sizeof(short));
    POTHOS_TEST_EQUAL(Pothos::DType("int").size(), sizeof(int));
    POTHOS_TEST_EQUAL(Pothos::DType("long").size(), sizeof(long));
    POTHOS_TEST_EQUAL(Pothos::DType("long long").size(), sizeof(long long));
    POTHOS_TEST_EQUAL(Pothos::DType("float").size(), sizeof(float));
    POTHOS_TEST_EQUAL(Pothos::DType("double").size(), sizeof(double));
    POTHOS_TEST_EQUAL(Pothos::DType("complex_float64").size(), sizeof(std::complex<double>));
    POTHOS_TEST_EQUAL(Pothos::DType("complex128").size(), sizeof(std::complex<double>));
}

POTHOS_TEST_BLOCK("/framework/tests", test_dtype_name)
{
    POTHOS_TEST_EQUAL(Pothos::DType().name(), "none");
    POTHOS_TEST_EQUAL(Pothos::DType("").name(), "none");
    POTHOS_TEST_EQUAL(Pothos::DType("custom").name(), "custom");
    POTHOS_TEST_EQUAL(Pothos::DType(typeid(int)).name(), "int32");
    POTHOS_TEST_EQUAL(Pothos::DType(typeid(signed int)).name(), "int32");
    POTHOS_TEST_EQUAL(Pothos::DType(typeid(unsigned int)).name(), "uint32");
    POTHOS_TEST_EQUAL(Pothos::DType(typeid(std::complex<short>)).name(), "complex_int16");
    POTHOS_TEST_EQUAL(Pothos::DType(typeid(std::complex<double>)).name(), "complex_float64");

    POTHOS_TEST_TRUE(Pothos::DType(Pothos::DType("").name()) == Pothos::DType(""));
    POTHOS_TEST_TRUE(Pothos::DType(Pothos::DType("custom").name()) == Pothos::DType("custom"));
    POTHOS_TEST_TRUE(Pothos::DType(Pothos::DType(typeid(int)).name()) == Pothos::DType(typeid(int)));
    POTHOS_TEST_TRUE(Pothos::DType(Pothos::DType(typeid(signed int)).name()) == Pothos::DType(typeid(signed int)));
    POTHOS_TEST_TRUE(Pothos::DType(Pothos::DType(typeid(unsigned int)).name()) == Pothos::DType(typeid(unsigned int)));
    POTHOS_TEST_TRUE(Pothos::DType(Pothos::DType(typeid(std::complex<short>)).name()) == Pothos::DType(typeid(std::complex<short>)));
    POTHOS_TEST_TRUE(Pothos::DType(Pothos::DType(typeid(std::complex<float>)).name()) == Pothos::DType(typeid(std::complex<float>)));
}


POTHOS_TEST_BLOCK("/framework/tests", test_dtype_dimensions)
{
    POTHOS_TEST_TRUE(Pothos::DType("int, 42") == Pothos::DType(typeid(int), 42));
    POTHOS_TEST_EQUAL(Pothos::DType("int, 42").size(), sizeof(int)*42);
    POTHOS_TEST_EQUAL(Pothos::DType("custom", 21).size(), 21);
    POTHOS_TEST_EQUAL(Pothos::DType("custom, 21").size(), 21);
    POTHOS_TEST_EQUAL(Pothos::DType("none, 21").size(), 0);
    POTHOS_TEST_EQUAL(Pothos::DType(", 21").size(), 0);
}

