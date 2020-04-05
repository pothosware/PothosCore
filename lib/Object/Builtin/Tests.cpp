// Copyright (c) 2013-2017 Josh Blum
//               2019-2020 Nicholas Corgan
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Object.hpp>
#include <Pothos/Testing.hpp>
#include <vector>
#include <complex>
#include <sstream>

class NeverHeardOfFooBar {};

POTHOS_TEST_BLOCK("/object/tests", test_object_equals)
{
    Pothos::Object nullObj;
    POTHOS_TEST_TRUE(nullObj == nullObj);
    POTHOS_TEST_FALSE(nullObj);
    POTHOS_TEST_TRUE(nullObj == Pothos::Object());

    Pothos::Object intObj(int(42));
    POTHOS_TEST_NOT_EQUAL(nullObj, intObj);

    Pothos::Object intCopy = intObj;
    POTHOS_TEST_TRUE(intObj == intCopy);

    Pothos::Object intCopy2; intCopy2 = intCopy;
    POTHOS_TEST_TRUE(intCopy == intCopy2);
}

POTHOS_TEST_BLOCK("/object/tests", test_object_value)
{
    Pothos::Object intObj(int(42));
    POTHOS_TEST_TRUE(intObj.type() == typeid(int));
    POTHOS_TEST_EQUAL(intObj.extract<int>(), 42);
}

POTHOS_TEST_BLOCK("/object/tests", test_object_throw)
{
    Pothos::Object nullObj;
    POTHOS_TEST_TRUE(nullObj.type() == typeid(Pothos::NullObject));
    POTHOS_TEST_THROWS(nullObj.extract<int>(), Pothos::ObjectConvertError);

    Pothos::Object intObj(int(42));
    POTHOS_TEST_THROWS(intObj.extract<char>(), Pothos::ObjectConvertError);
}

POTHOS_TEST_BLOCK("/object/tests", test_object_mutable)
{
    Pothos::Object intObj(int(42));
    POTHOS_TEST_EQUAL(intObj.ref<int>(), 42);

    intObj.ref<int>() = 21;
    POTHOS_TEST_EQUAL(intObj.ref<int>(), 21);

    //too many references, non-const reference denied
    POTHOS_TEST_TRUE(intObj.unique());
    Pothos::Object intObjCopy = intObj;
    POTHOS_TEST_FALSE(intObj.unique());
    POTHOS_TEST_FALSE(intObjCopy.unique());
    POTHOS_TEST_THROWS(intObj.ref<int>(), Pothos::ObjectConvertError);
}

Pothos::Object someFunctionTakesObject(const Pothos::Object &obj)
{
    return obj;
}

POTHOS_TEST_BLOCK("/object/tests", test_convert_numbers)
{
    Pothos::Object intObj(int(42));
    const long longVal = intObj;
    POTHOS_TEST_EQUAL(longVal, 42);
    POTHOS_TEST_THROWS(intObj.convert<NeverHeardOfFooBar>(), Pothos::ObjectConvertError);

    //tests for canConvert
    POTHOS_TEST_TRUE(intObj.canConvert(typeid(int)));
    POTHOS_TEST_TRUE(intObj.canConvert(typeid(long)));
    POTHOS_TEST_FALSE(intObj.canConvert(typeid(NeverHeardOfFooBar)));

    //tests bool explicit and implicit
    Pothos::Object trueObj(true);
    POTHOS_TEST_TRUE(trueObj.convert<bool>());
    const bool trueRes = trueObj;
    POTHOS_TEST_TRUE(trueRes);

    Pothos::Object falseObj(false);
    POTHOS_TEST_FALSE(falseObj.convert<bool>());
    const bool falseRes = falseObj;
    POTHOS_TEST_FALSE(falseRes);

    //test int to double
    POTHOS_TEST_EQUAL(double(Pothos::Object(+1)), +1.0);
    POTHOS_TEST_EQUAL(double(Pothos::Object(-1)), -1.0);
    POTHOS_TEST_EQUAL(double(Pothos::Object(0)), 0.0);

    //tests for range errors
    POTHOS_TEST_THROWS(Pothos::Object(-1).convert<unsigned>(), Pothos::RangeException);
    POTHOS_TEST_THROWS(Pothos::Object(1024).convert<char>(), Pothos::RangeException);
    POTHOS_TEST_THROWS(Pothos::Object(-1024).convert<char>(), Pothos::RangeException);
}

POTHOS_TEST_BLOCK("/object/tests", test_convert_complex)
{
    Pothos::Object complexObj(std::complex<double>(2, -3));
    POTHOS_TEST_EQUAL(complexObj.convert<std::complex<float>>(), std::complex<float>(2, -3));
    POTHOS_TEST_THROWS(complexObj.convert<int>(), Pothos::RangeException);
}

POTHOS_TEST_BLOCK("/object/tests", test_convert_vectors)
{
    std::vector<unsigned int> inputVec;
    inputVec.push_back(1);
    inputVec.push_back(2);
    inputVec.push_back(3);
    Pothos::Object inputVecObj(inputVec);

    std::vector<unsigned long> outputVec = inputVecObj;

    POTHOS_TEST_EQUALV(inputVec, outputVec);
}

POTHOS_TEST_BLOCK("/object/tests", test_serialize_null)
{
    Pothos::Object null0;
    std::stringstream ss;
    null0.serialize(ss);

    Pothos::Object null1;
    null1.deserialize(ss);

    POTHOS_TEST_FALSE(null1);
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
    POTHOS_TEST_GT(num0, num1);
    POTHOS_TEST_TRUE(num1 < num0);
}
