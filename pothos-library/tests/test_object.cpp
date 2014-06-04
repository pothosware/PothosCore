// Copyright (c) 2013-2013 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <boost/test/unit_test.hpp>
#include <stdexcept>
#include <iostream>

#include <Pothos/Object.hpp>

BOOST_AUTO_TEST_CASE(test_object_equals)
{
    Pothos::Object nullObj;
    BOOST_CHECK(nullObj == nullObj);
    BOOST_CHECK(not nullObj);
    BOOST_CHECK(nullObj == Pothos::Object());

    Pothos::Object intObj(int(42));
    BOOST_CHECK(not (nullObj == intObj));

    Pothos::Object intCopy = intObj;
    BOOST_CHECK(intObj == intCopy);

    Pothos::Object intCopy2; intCopy2 = intCopy;
    BOOST_CHECK(intCopy == intCopy2);
}

BOOST_AUTO_TEST_CASE(test_object_value)
{
    Pothos::Object intObj(int(42));
    BOOST_REQUIRE(intObj.type() == typeid(int));
    BOOST_CHECK_EQUAL(intObj.extract<int>(), 42);
}

BOOST_AUTO_TEST_CASE(test_object_throw)
{
    Pothos::Object nullObj;
    BOOST_CHECK(nullObj.type() == typeid(Pothos::NullObject));
    BOOST_CHECK_THROW(nullObj.extract<int>(), Pothos::ObjectConvertError);

    Pothos::Object intObj(int(42));
    BOOST_CHECK_THROW(intObj.extract<char>(), Pothos::ObjectConvertError);
}

BOOST_AUTO_TEST_CASE(test_object_mutable)
{
    Pothos::ObjectM nullObj;
    BOOST_CHECK(not nullObj);
    BOOST_CHECK(nullObj == Pothos::Object());

    Pothos::ObjectM intObj(int(42));
    BOOST_CHECK_EQUAL(intObj.extract<int>(), 42);

    intObj.extract<int>() = 21;
    BOOST_CHECK_EQUAL(intObj.extract<int>(), 21);
}

Pothos::Object someFunctionTakesObject(const Pothos::Object &obj)
{
    return obj;
}

BOOST_AUTO_TEST_CASE(test_object_mutable_copy_assigns)
{
    Pothos::ObjectM objM0(int(0));
    Pothos::ObjectM objM1(int(1));
    Pothos::Object obj0 = someFunctionTakesObject(objM0);
    Pothos::Object obj1; obj1 = objM1;
    BOOST_CHECK_EQUAL(obj0.extract<int>(), 0);
    BOOST_CHECK_EQUAL(obj1.extract<int>(), 1);

    Pothos::ObjectM objM0Copy = objM0;
    Pothos::ObjectM objM1Copy; objM1Copy = objM1;
    BOOST_CHECK_EQUAL(objM0Copy.extract<int>(), 0);
    BOOST_CHECK_EQUAL(objM1Copy.extract<int>(), 1);
}
