// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <boost/test/unit_test.hpp>
#include <stdexcept>
#include <iostream>

#include <Pothos/Callable.hpp>
#include <vector>
#include <string>

struct TestClass
{

    TestClass(void)
    {
        _bar = -1;
    }

    TestClass(const int bar)
    {
        _bar = bar;
    }

    TestClass(const long bar, const std::string &)
    {
        _bar = bar;
    }

    void setBar(const int bar)
    {
        _bar = bar;
    }

    int getBar(void)
    {
        return _bar;
    }

    static long strLen(const std::string &s)
    {
        return s.size();
    }

    static long add(int a, unsigned b)
    {
        return long(a + b);
    }

    static long long addMany(int a, unsigned b, long c, char d, short e)
    {
        return (long long)(a + b + c + d + e);
    }

    static int itsGonnaThrow(const int &)
    {
        throw std::runtime_error("told you so");
    }

    int overloaded(const int)
    {
        return 0;
    }

    int overloaded(const long &)
    {
        return 1;
    }

    void overloaded(const std::string &)
    {
        return;
    }

    int _bar;
};

struct NonsenseClass
{
    //nothing here
};

BOOST_AUTO_TEST_CASE(test_callable_null)
{
    Pothos::Callable callNull;
    BOOST_CHECK_THROW(callNull.call(0), Pothos::CallableNullError);
    BOOST_CHECK(callNull == callNull);
    BOOST_CHECK(callNull == Pothos::Callable());
}

/***********************************************************************
 * Passing object types in and out
 **********************************************************************/
static Pothos::Object getObj(void)
{
    return Pothos::Object(int(42));
}

static Pothos::Object passObj(const Pothos::Object &o)
{
    return o;
}

BOOST_AUTO_TEST_CASE(test_callable_meta)
{
    Pothos::Callable getObjCall(&getObj);
    Pothos::Object ret = getObjCall.opaqueCall(nullptr, 0);
    BOOST_CHECK(ret.type() == typeid(Pothos::Object));
    BOOST_CHECK(ret.extract<Pothos::Object>().type() == typeid(int));
    BOOST_CHECK_EQUAL(ret.extract<Pothos::Object>().extract<int>(), 42);

    Pothos::Callable passObjCall(&passObj);
    Pothos::Object ret2 = getObjCall.opaqueCall(&ret, 0);
    BOOST_CHECK(ret2.type() == typeid(Pothos::Object));
    BOOST_CHECK(ret2.extract<Pothos::Object>().type() == typeid(int));
    BOOST_CHECK_EQUAL(ret2.extract<Pothos::Object>().extract<int>(), 42);
}

/***********************************************************************
 * Test binding methods
 **********************************************************************/
BOOST_AUTO_TEST_CASE(test_callable_with_methods)
{
    //bind some class methods
    Pothos::Callable setBar(&TestClass::setBar);
    BOOST_CHECK_EQUAL(setBar.getNumArgs(), 2);
    BOOST_CHECK(setBar.type(-1) == typeid(void));
    BOOST_CHECK(setBar.type(0) == typeid(TestClass));
    BOOST_CHECK(setBar.type(1) == typeid(int));
    BOOST_CHECK_THROW(setBar.call(0), Pothos::CallableArgumentError);

    Pothos::Callable getBar(&TestClass::getBar);
    BOOST_CHECK_EQUAL(getBar.getNumArgs(), 1);
    BOOST_CHECK(getBar.type(-1) == typeid(int));
    BOOST_CHECK(getBar.type(0) == typeid(TestClass));
    BOOST_CHECK_THROW(getBar.call<int>(), Pothos::CallableArgumentError);

    //call the class methods
    TestClass test;
    setBar.call(std::ref(test), int(42));
    BOOST_CHECK_EQUAL(42, getBar.call<int>(std::ref(test)));

    //check the return error conditions
    BOOST_CHECK_THROW(setBar.call<int>(std::ref(test), 21), Pothos::CallableReturnError);
    BOOST_CHECK_THROW(getBar.call<NonsenseClass>(std::ref(test)), Pothos::CallableReturnError);
}

/***********************************************************************
 * Test binding functions
 **********************************************************************/
BOOST_AUTO_TEST_CASE(test_callable_with_functions)
{
    //bind a function (static method)
    Pothos::Callable strLen(&TestClass::strLen);
    BOOST_CHECK_EQUAL(strLen.getNumArgs(), 1);
    BOOST_CHECK(strLen.type(-1) == typeid(long));
    BOOST_CHECK(strLen.type(0) == typeid(std::string));
    BOOST_CHECK_EQUAL(5, strLen.call<long>(std::string("hello")));
    BOOST_CHECK_THROW(strLen.call<long>(NonsenseClass()), Pothos::CallableArgumentError);

    //test copy ability
    Pothos::Callable strLenCopy0 = strLen;
    BOOST_CHECK_EQUAL(5, strLenCopy0.call<long>(std::string("world")));

    Pothos::Callable strLenCopy1 = Pothos::Callable(strLen);
    BOOST_CHECK_EQUAL(2, strLenCopy1.call<long>(std::string("!!")));

    //test multiple args
    Pothos::Callable add(&TestClass::add);
    BOOST_CHECK_EQUAL(32, add.call<long>(int(10), unsigned(22)));
    std::cout << add.toString() << std::endl;
}

/***********************************************************************
 * Test binding constructors
 **********************************************************************/
BOOST_AUTO_TEST_CASE(test_callable_constructors)
{
    auto constructor0 = Pothos::Callable::factory<TestClass>();
    BOOST_CHECK(constructor0.type(-1) == typeid(TestClass));

    auto constructor1 = Pothos::Callable::factory<TestClass, const int>();
    BOOST_CHECK(constructor1.type(-1) == typeid(TestClass));
    BOOST_CHECK(constructor1.type(0) == typeid(int));

    auto constructor2 = Pothos::Callable::factory<TestClass, long, std::string>();
    BOOST_CHECK(constructor2.type(-1) == typeid(TestClass));
    BOOST_CHECK(constructor2.type(0) == typeid(long));
    BOOST_CHECK(constructor2.type(1) == typeid(std::string));

    TestClass test0 = constructor0.call<TestClass>();
    BOOST_CHECK_EQUAL(test0.getBar(), int(-1));
    TestClass test1 = constructor1.call<TestClass>(int(42));
    BOOST_CHECK_EQUAL(test1.getBar(), int(42));
    TestClass test2 = constructor1.call<TestClass>(long(21), std::string("test"));
    BOOST_CHECK_EQUAL(test2.getBar(), int(21));
}

/***********************************************************************
 * Test binding overloaded
 **********************************************************************/
BOOST_AUTO_TEST_CASE(test_callable_overloaded)
{
    //bind a function (static method)
    auto overloaded0 = Pothos::Callable::make<const int>(&TestClass::overloaded);
    auto overloaded1 = Pothos::Callable::make<const long &>(&TestClass::overloaded);
    auto overloaded2 = Pothos::Callable::make<const std::string &>(&TestClass::overloaded);

    BOOST_CHECK(overloaded0.type(-1) == typeid(int));
    BOOST_CHECK(overloaded1.type(-1) == typeid(int));
    BOOST_CHECK(overloaded2.type(-1) == typeid(void));

    TestClass test;
    BOOST_CHECK_EQUAL(overloaded0.call<int>(std::ref(test), int(0)), 0);
    BOOST_CHECK_EQUAL(overloaded1.call<int>(std::ref(test), long(0)), 1);
}

/***********************************************************************
 * Test binding arguments
 **********************************************************************/
BOOST_AUTO_TEST_CASE(test_callable_bind)
{
    //bind the class instance into set and get
    Pothos::Callable setBar(&TestClass::setBar);
    Pothos::Callable getBar(&TestClass::getBar);
    TestClass test;
    setBar.bind(std::ref(test), 0);
    getBar.bind(std::ref(test), 0);
    setBar.call(int(42));
    BOOST_CHECK_EQUAL(42, getBar.call<int>());

    //bind and unbind arguments for add
    Pothos::Callable add(&TestClass::add);
    add.bind(unsigned(11), 1);
    BOOST_CHECK_EQUAL(21, add.call<long>(int(10)));
    add.bind(int(33), 0);
    BOOST_CHECK_EQUAL(44, add.call<long>());
    add.unbind(1);
    BOOST_CHECK_EQUAL(43, add.call<long>(unsigned(10)));

    //test type() and numArgs() logic with many args
    Pothos::Callable addMany(&TestClass::addMany);
    std::cout << addMany.toString() << std::endl;
    BOOST_CHECK_EQUAL(addMany.getNumArgs(), 5);
    BOOST_CHECK(addMany.type(-1) == typeid(long long));
    BOOST_CHECK(addMany.type(0) == typeid(int));
    BOOST_CHECK(addMany.type(1) == typeid(unsigned));
    BOOST_CHECK(addMany.type(2) == typeid(long));
    BOOST_CHECK(addMany.type(3) == typeid(char));
    BOOST_CHECK(addMany.type(4) == typeid(short));
    BOOST_CHECK_THROW(addMany.type(5), Pothos::CallableArgumentError);

    //bind something, recheck type() and numArgs()
    addMany.bind(long(42), 2);
    BOOST_CHECK_EQUAL(addMany.getNumArgs(), 4);
    BOOST_CHECK(addMany.type(-1) == typeid(long long));
    BOOST_CHECK(addMany.type(0) == typeid(int));
    BOOST_CHECK(addMany.type(1) == typeid(unsigned));
    BOOST_CHECK(addMany.type(2) == typeid(char));
    BOOST_CHECK(addMany.type(3) == typeid(short));
    BOOST_CHECK_THROW(addMany.type(4), Pothos::CallableArgumentError);

    //bind something, recheck type() and numArgs()
    addMany.bind(long(21), 0);
    BOOST_CHECK_EQUAL(addMany.getNumArgs(), 3);
    BOOST_CHECK(addMany.type(-1) == typeid(long long));
    BOOST_CHECK(addMany.type(0) == typeid(unsigned));
    BOOST_CHECK(addMany.type(1) == typeid(char));
    BOOST_CHECK(addMany.type(2) == typeid(short));
    BOOST_CHECK_THROW(addMany.type(3), Pothos::CallableArgumentError);
}

/***********************************************************************
 * Test throwing
 **********************************************************************/
BOOST_AUTO_TEST_CASE(test_callable_throwing)
{
    Pothos::Callable itsGonnaThrow(&TestClass::itsGonnaThrow);
    BOOST_CHECK_THROW(itsGonnaThrow.call(int(42)), std::runtime_error);
}
