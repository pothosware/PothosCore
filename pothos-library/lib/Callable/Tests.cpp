// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Callable.hpp>
#include <Pothos/Testing.hpp>
#include <string>
#include <iostream>

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

POTHOS_TEST_BLOCK("/callable/tests", test_callable_null)
{
    Pothos::Callable callNull;
    POTHOS_TEST_THROWS(callNull.callVoid(0), Pothos::CallableNullError);
    POTHOS_TEST_TRUE(callNull == callNull);
    POTHOS_TEST_TRUE(callNull == Pothos::Callable());
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

POTHOS_TEST_BLOCK("/callable/tests", test_callable_meta)
{
    Pothos::Callable getObjCall(&getObj);
    Pothos::Object ret = getObjCall.opaqueCall(nullptr, 0);
    POTHOS_TEST_TRUE(ret.type() == typeid(Pothos::Object));
    POTHOS_TEST_TRUE(ret.extract<Pothos::Object>().type() == typeid(int));
    POTHOS_TEST_EQUAL(ret.extract<Pothos::Object>().extract<int>(), 42);

    Pothos::Callable passObjCall(&passObj);
    Pothos::Object ret2 = getObjCall.opaqueCall(&ret, 0);
    POTHOS_TEST_TRUE(ret2.type() == typeid(Pothos::Object));
    POTHOS_TEST_TRUE(ret2.extract<Pothos::Object>().type() == typeid(int));
    POTHOS_TEST_EQUAL(ret2.extract<Pothos::Object>().extract<int>(), 42);
}

/***********************************************************************
 * Test binding methods
 **********************************************************************/
POTHOS_TEST_BLOCK("/callable/tests", test_callable_with_methods)
{
    //bind some class methods
    Pothos::Callable setBar(&TestClass::setBar);
    POTHOS_TEST_EQUAL(setBar.getNumArgs(), 2);
    POTHOS_TEST_TRUE(setBar.type(-1) == typeid(void));
    POTHOS_TEST_TRUE(setBar.type(0) == typeid(TestClass));
    POTHOS_TEST_TRUE(setBar.type(1) == typeid(int));
    POTHOS_TEST_THROWS(setBar.callVoid(0), Pothos::CallableArgumentError);

    Pothos::Callable getBar(&TestClass::getBar);
    POTHOS_TEST_EQUAL(getBar.getNumArgs(), 1);
    POTHOS_TEST_TRUE(getBar.type(-1) == typeid(int));
    POTHOS_TEST_TRUE(getBar.type(0) == typeid(TestClass));
    POTHOS_TEST_THROWS(getBar.call<int>(), Pothos::CallableArgumentError);

    //call the class methods
    TestClass test;
    setBar.callVoid(std::ref(test), int(42));
    POTHOS_TEST_EQUAL(42, getBar.call<int>(std::ref(test)));

    //check the return error conditions
    POTHOS_TEST_THROWS(setBar.call<int>(std::ref(test), 21), Pothos::CallableReturnError);
    POTHOS_TEST_THROWS(getBar.call<NonsenseClass>(std::ref(test)), Pothos::CallableReturnError);
}

/***********************************************************************
 * Test binding functions
 **********************************************************************/
POTHOS_TEST_BLOCK("/callable/tests", test_callable_with_functions)
{
    //bind a function (static method)
    Pothos::Callable strLen(&TestClass::strLen);
    POTHOS_TEST_EQUAL(strLen.getNumArgs(), 1);
    POTHOS_TEST_TRUE(strLen.type(-1) == typeid(long));
    POTHOS_TEST_TRUE(strLen.type(0) == typeid(std::string));
    POTHOS_TEST_EQUAL(5, strLen.call<long>(std::string("hello")));
    POTHOS_TEST_THROWS(strLen.call<long>(NonsenseClass()), Pothos::CallableArgumentError);

    //test copy ability
    Pothos::Callable strLenCopy0 = strLen;
    POTHOS_TEST_EQUAL(5, strLenCopy0.call<long>(std::string("world")));

    Pothos::Callable strLenCopy1 = Pothos::Callable(strLen);
    POTHOS_TEST_EQUAL(2, strLenCopy1.call<long>(std::string("!!")));

    //test multiple args
    Pothos::Callable add(&TestClass::add);
    POTHOS_TEST_EQUAL(32, add.call<long>(int(10), unsigned(22)));
    std::cout << add.toString() << std::endl;
}

/***********************************************************************
 * Test binding constructors
 **********************************************************************/
POTHOS_TEST_BLOCK("/callable/tests", test_callable_constructors)
{
    auto constructor0 = Pothos::Callable::factory<TestClass>();
    POTHOS_TEST_TRUE(constructor0.type(-1) == typeid(TestClass));

    auto constructor1 = Pothos::Callable::factory<TestClass, const int>();
    POTHOS_TEST_TRUE(constructor1.type(-1) == typeid(TestClass));
    POTHOS_TEST_TRUE(constructor1.type(0) == typeid(int));

    auto constructor2 = Pothos::Callable::factory<TestClass, long, std::string>();
    POTHOS_TEST_TRUE(constructor2.type(-1) == typeid(TestClass));
    POTHOS_TEST_TRUE(constructor2.type(0) == typeid(long));
    POTHOS_TEST_TRUE(constructor2.type(1) == typeid(std::string));

    TestClass test0 = constructor0.call<TestClass>();
    POTHOS_TEST_EQUAL(test0.getBar(), int(-1));
    TestClass test1 = constructor1.call<TestClass>(int(42));
    POTHOS_TEST_EQUAL(test1.getBar(), int(42));
    TestClass test2 = constructor1.call<TestClass>(long(21), std::string("test"));
    POTHOS_TEST_EQUAL(test2.getBar(), int(21));
}

/***********************************************************************
 * Test binding overloaded
 **********************************************************************/
POTHOS_TEST_BLOCK("/callable/tests", test_callable_overloaded)
{
    //bind a function (static method)
    auto overloaded0 = Pothos::Callable::make<const int>(&TestClass::overloaded);
    auto overloaded1 = Pothos::Callable::make<const long &>(&TestClass::overloaded);
    auto overloaded2 = Pothos::Callable::make<const std::string &>(&TestClass::overloaded);

    POTHOS_TEST_TRUE(overloaded0.type(-1) == typeid(int));
    POTHOS_TEST_TRUE(overloaded1.type(-1) == typeid(int));
    POTHOS_TEST_TRUE(overloaded2.type(-1) == typeid(void));

    TestClass test;
    POTHOS_TEST_EQUAL(overloaded0.call<int>(std::ref(test), int(0)), 0);
    POTHOS_TEST_EQUAL(overloaded1.call<int>(std::ref(test), long(0)), 1);
}

/***********************************************************************
 * Test binding arguments
 **********************************************************************/
POTHOS_TEST_BLOCK("/callable/tests", test_callable_bind)
{
    //bind the class instance into set and get
    Pothos::Callable setBar(&TestClass::setBar);
    Pothos::Callable getBar(&TestClass::getBar);
    TestClass test;
    setBar.bind(std::ref(test), 0);
    getBar.bind(std::ref(test), 0);
    setBar.callVoid(int(42));
    POTHOS_TEST_EQUAL(42, getBar.call<int>());

    //bind and unbind arguments for add
    Pothos::Callable add(&TestClass::add);
    add.bind(unsigned(11), 1);
    POTHOS_TEST_EQUAL(21, add.call<long>(int(10)));
    add.bind(int(33), 0);
    POTHOS_TEST_EQUAL(44, add.call<long>());
    add.unbind(1);
    POTHOS_TEST_EQUAL(43, add.call<long>(unsigned(10)));

    //test type() and numArgs() logic with many args
    Pothos::Callable addMany(&TestClass::addMany);
    std::cout << addMany.toString() << std::endl;
    POTHOS_TEST_EQUAL(addMany.getNumArgs(), 5);
    POTHOS_TEST_TRUE(addMany.type(-1) == typeid(long long));
    POTHOS_TEST_TRUE(addMany.type(0) == typeid(int));
    POTHOS_TEST_TRUE(addMany.type(1) == typeid(unsigned));
    POTHOS_TEST_TRUE(addMany.type(2) == typeid(long));
    POTHOS_TEST_TRUE(addMany.type(3) == typeid(char));
    POTHOS_TEST_TRUE(addMany.type(4) == typeid(short));
    POTHOS_TEST_THROWS(addMany.type(5), Pothos::CallableArgumentError);

    //bind something, recheck type() and numArgs()
    addMany.bind(long(42), 2);
    POTHOS_TEST_EQUAL(addMany.getNumArgs(), 4);
    POTHOS_TEST_TRUE(addMany.type(-1) == typeid(long long));
    POTHOS_TEST_TRUE(addMany.type(0) == typeid(int));
    POTHOS_TEST_TRUE(addMany.type(1) == typeid(unsigned));
    POTHOS_TEST_TRUE(addMany.type(2) == typeid(char));
    POTHOS_TEST_TRUE(addMany.type(3) == typeid(short));
    POTHOS_TEST_THROWS(addMany.type(4), Pothos::CallableArgumentError);

    //bind something, recheck type() and numArgs()
    addMany.bind(long(21), 0);
    POTHOS_TEST_EQUAL(addMany.getNumArgs(), 3);
    POTHOS_TEST_TRUE(addMany.type(-1) == typeid(long long));
    POTHOS_TEST_TRUE(addMany.type(0) == typeid(unsigned));
    POTHOS_TEST_TRUE(addMany.type(1) == typeid(char));
    POTHOS_TEST_TRUE(addMany.type(2) == typeid(short));
    POTHOS_TEST_THROWS(addMany.type(3), Pothos::CallableArgumentError);
}

/***********************************************************************
 * Test throwing
 **********************************************************************/
POTHOS_TEST_BLOCK("/callable/tests", test_callable_throwing)
{
    Pothos::Callable itsGonnaThrow(&TestClass::itsGonnaThrow);
    POTHOS_TEST_THROWS(itsGonnaThrow.callVoid(int(42)), std::runtime_error);
}
