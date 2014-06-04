// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Object.hpp>
#include <Pothos/Testing.hpp>
#include <vector>
#include <complex>
#include <sstream>

class NeverHeardOfFooBar {};

POTHOS_TEST_BLOCK("/object/tests", test_convert_numbers)
{
    Pothos::Object intObj(int(42));
    const long longVal = intObj.convert<long>();
    POTHOS_TEST_EQUAL(longVal, 42);
    POTHOS_TEST_THROWS(intObj.convert<NeverHeardOfFooBar>(), Pothos::ObjectConvertError);

    //tests for canConvert
    POTHOS_TEST_TRUE(intObj.canConvert(typeid(int)));
    POTHOS_TEST_TRUE(intObj.canConvert(typeid(long)));
    POTHOS_TEST_TRUE(not intObj.canConvert(typeid(NeverHeardOfFooBar)));
}

POTHOS_TEST_BLOCK("/object/tests", test_convert_complex)
{
    Pothos::Object complexObj(std::complex<double>(2, -3));
    POTHOS_TEST_EQUAL(complexObj.convert<std::complex<float>>(), std::complex<float>(2, -3));
    POTHOS_TEST_EQUAL(complexObj.convert<int>(), 2);
}

POTHOS_TEST_BLOCK("/object/tests", test_convert_vectors)
{
    std::vector<unsigned int> inputVec;
    inputVec.push_back(1);
    inputVec.push_back(2);
    inputVec.push_back(3);
    Pothos::Object inputVecObj(inputVec);

    auto outputVec = inputVecObj.convert<std::vector<unsigned long>>();

    POTHOS_TEST_EQUALV(inputVec, outputVec);
}

POTHOS_TEST_BLOCK("/object/tests", test_serialize_null)
{
    Pothos::Object null0;
    std::stringstream ss;
    null0.serialize(ss);

    Pothos::Object null1;
    null1.deserialize(ss);

    POTHOS_TEST_TRUE(not null1);
}

POTHOS_TEST_BLOCK("/object/tests", test_serialize_int)
{
    Pothos::Object int0(42);
    std::stringstream ss;
    int0.serialize(ss);

    Pothos::Object int1;
    int1.deserialize(ss);

    POTHOS_TEST_TRUE(int1);
    POTHOS_TEST_TRUE(int1.type() == typeid(int));
    POTHOS_TEST_EQUAL(int1.extract<int>(), 42);
}

POTHOS_TEST_BLOCK("/object/tests", test_serialize_obj)
{
    Pothos::Object int0(42);
    Pothos::Object intObj0 = Pothos::Object::make(int0);
    std::stringstream ss;
    intObj0.serialize(ss);

    Pothos::Object intObj1;
    intObj1.deserialize(ss);
    POTHOS_TEST_TRUE(intObj1);
    POTHOS_TEST_TRUE(intObj1.type() == typeid(Pothos::Object));

    Pothos::Object int1 = intObj1.extract<Pothos::Object>();
    POTHOS_TEST_TRUE(int1);
    POTHOS_TEST_TRUE(int1.type() == typeid(int));
    POTHOS_TEST_EQUAL(int1.extract<int>(), 42);
}

POTHOS_TEST_BLOCK("/object/tests", test_compare_to)
{
    Pothos::Object null0;
    Pothos::Object null1;
    POTHOS_TEST_EQUAL(null0.compareTo(null1), 0);

    Pothos::Object s0("hello");
    Pothos::Object s1("world");
    POTHOS_TEST_EQUAL(s0.compareTo(s1), std::string("hello").compare("world"));

    POTHOS_TEST_THROWS(null0.compareTo(s0), Pothos::ObjectCompareError);
    POTHOS_TEST_THROWS(null1.compareTo(s1), Pothos::ObjectCompareError);

    Pothos::Object num0(long(42));
    Pothos::Object num1(double(-21));
    POTHOS_TEST_TRUE(num0 > num1);
    POTHOS_TEST_TRUE(num1 < num0);
}
