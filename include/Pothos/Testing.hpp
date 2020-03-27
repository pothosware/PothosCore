///
/// \file Testing.hpp
///
/// Macros for creating self-test plugins.
/// Self tests are installed into the registry and executed at runtime.
/// The test macros are loosely based on the Boost unit test suite.
///
/// \copyright
/// Copyright (c) 2013-2017 Josh Blum
///                    2020 Nicholas Corgan
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Object.hpp>
#include <Pothos/Plugin.hpp>
#include <Pothos/Callable.hpp>
#include <Pothos/Exception.hpp>
#include <cmath>
#include <memory>
#include <string>

/*!
 * Declare a unit test block of code inside a plugin.
 *
 * The unit test will be installed into /path/name in the PluginRegistry.
 * Once installed, this unit test can be checked with the test utility.
 *
 * \param path a valid PluginPath
 * \param name a valid function name
 *
 * Example usage:
 * \code
 * POTHOS_TEST_BLOCK("/sys/foo", test_bar)
 * {
 *     POTHOS_TEST_EQUAL(etc...);
 * }
 * \endcode
 */
#define POTHOS_TEST_BLOCK(path, name) \
    POTHOS_STATIC_FIXTURE_DECL void name ## Runner(void); \
    template <Pothos::Detail::InitFcn runner> \
    struct name : Pothos::TestingBase \
    { \
        void runTestsImpl(void) \
        { \
            POTHOS_TEST_CHECKPOINT(); \
            runner(); \
        } \
    }; \
    pothos_static_block(name) \
    { \
        std::shared_ptr<Pothos::TestingBase> testObj(new name<name ## Runner>()); \
        Pothos::PluginRegistry::add(Pothos::Plugin( \
            Pothos::PluginPath(path).join(#name), Pothos::Object(testObj))); \
    } \
    POTHOS_STATIC_FIXTURE_DECL void name ## Runner(void)

//! Checkpoint macro to track last successful line executed
#define POTHOS_TEST_CHECKPOINT() \
    Pothos::TestingBase::current().report("checkpoint", "", "", __LINE__, __FILE__)

//! Private macro to test a statement for exceptions
#define __POTHOS_TEST_STATEMENT(message, statement) \
{ \
    Pothos::TestingBase::current().report(message, #statement, "", __LINE__, __FILE__); \
    try{statement;} \
    catch(const std::string &ex){Pothos::TestingBase::current().report(message, #statement, ex, __LINE__, __FILE__);} \
    catch(const Pothos::Exception &ex){Pothos::TestingBase::current().report(message, #statement, ex.displayText(), __LINE__, __FILE__);} \
    catch(const std::exception &ex){Pothos::TestingBase::current().report(message, #statement, ex.what(), __LINE__, __FILE__);} \
    catch(...){Pothos::TestingBase::current().report(message, #statement, "unknown", __LINE__, __FILE__);} \
}

//! Private macro to assert on a statement's truth
#define __POTHOS_TEST_ASSERT(message, statement) \
{ \
    __POTHOS_TEST_STATEMENT(message, if (statement) {} else throw std::string("statement \"" #statement "\" evaluated false");); \
}

//! Private macro to assert on a statement's falsehood
#define __POTHOS_TEST_ASSERT_FALSE(message, statement) \
{ \
    __POTHOS_TEST_STATEMENT(message, if (!(statement)) {} else throw std::string("statement \"" #statement "\" evaluated true");); \
}

//
// Basic assertions
//

//! Test if statement is true
#define POTHOS_TEST_TRUE(statement) \
{ \
    __POTHOS_TEST_ASSERT("assert true " #statement, statement); \
}

//! Test if statement is false
#define POTHOS_TEST_FALSE(statement) \
{ \
    __POTHOS_TEST_ASSERT_FALSE("assert false " #statement, statement); \
}

//
// Scalar comparisons
//

//! Test if two values are equal
#define POTHOS_TEST_EQUAL(lhs, rhs) \
{ \
    __POTHOS_TEST_ASSERT( \
        "assert equal " + Pothos::TestingBase::current().toString(lhs) + \
        " == " + Pothos::TestingBase::current().toString(rhs), (lhs) == (rhs)); \
}

//! Test if lhs > rhs
#define POTHOS_TEST_GT(lhs, rhs) \
{ \
    __POTHOS_TEST_ASSERT( \
        "assert " + Pothos::TestingBase::current().toString(lhs) + \
        " > " + Pothos::TestingBase::current().toString(rhs), (lhs) > (rhs)); \
}

//! Test if lhs >= rhs
#define POTHOS_TEST_GE(lhs, rhs) \
{ \
    __POTHOS_TEST_ASSERT( \
        "assert " + Pothos::TestingBase::current().toString(lhs) + \
        " >= " + Pothos::TestingBase::current().toString(rhs), (lhs) >= (rhs)); \
}

//! Test if lhs < rhs
#define POTHOS_TEST_LT(lhs, rhs) \
{ \
    __POTHOS_TEST_ASSERT( \
        "assert " + Pothos::TestingBase::current().toString(lhs) + \
        " < " + Pothos::TestingBase::current().toString(rhs), (lhs) < (rhs)); \
}

//! Test if lhs <= rhs
#define POTHOS_TEST_LE(lhs, rhs) \
{ \
    __POTHOS_TEST_ASSERT( \
        "assert " + Pothos::TestingBase::current().toString(lhs) + \
        " <= " + Pothos::TestingBase::current().toString(rhs), (lhs) <= (rhs)); \
}

//! Test if two values are equal within tolerance
#define POTHOS_TEST_CLOSE(lhs, rhs, tol) \
{ \
    __POTHOS_TEST_ASSERT( \
        "assert close " + Pothos::TestingBase::current().toString(lhs) + \
        " ~= " + Pothos::TestingBase::current().toString(rhs), (std::abs((lhs) - (rhs)) <= (tol))); \
}

//! Test if two values are not equal
#define POTHOS_TEST_NOT_EQUAL(lhs, rhs) \
{ \
    __POTHOS_TEST_ASSERT( \
        "assert not equal " + Pothos::TestingBase::current().toString(lhs) + \
        " != " + Pothos::TestingBase::current().toString(rhs), (lhs) != (rhs)); \
}

//! Test if two values are not equal within tolerance
#define POTHOS_TEST_NOT_CLOSE(lhs, rhs, tol) \
{ \
    __POTHOS_TEST_ASSERT( \
        "assert close " + Pothos::TestingBase::current().toString(lhs) + \
        " !~= " + Pothos::TestingBase::current().toString(rhs), (std::abs((lhs) - (rhs)) > (tol))); \
}

//
// Vector comparisons
//

//! Test two vectors for equality
#define POTHOS_TEST_EQUALV(lhs, rhs) \
{ \
    POTHOS_TEST_EQUAL((lhs).size(), (rhs).size()); \
    for (size_t i = 0; i < (lhs).size(); i++) \
    { \
        __POTHOS_TEST_ASSERT( \
            "index " + Pothos::TestingBase::current().toString(i) + \
            " asserts " + Pothos::TestingBase::current().toString((lhs)[i]) + \
            " == " + Pothos::TestingBase::current().toString((rhs)[i]), (lhs)[i] == (rhs)[i]); \
    } \
}

//! Test two vectors for equality within tolerance
#define POTHOS_TEST_CLOSEV(lhs, rhs, tol) \
{ \
    POTHOS_TEST_EQUAL((lhs).size(), (rhs).size()); \
    for (size_t i = 0; i < (lhs).size(); i++) \
    { \
        __POTHOS_TEST_ASSERT( \
            "index " + Pothos::TestingBase::current().toString(i) + \
            " asserts " + Pothos::TestingBase::current().toString((lhs)[i]) + \
            " ~= " + Pothos::TestingBase::current().toString((rhs)[i]), (std::abs((lhs)[i] - (rhs)[i]) <= (tol))); \
    } \
}

//! Test two vectors for nonequality
#define POTHOS_TEST_NOT_EQUALV(lhs, rhs) \
{ \
    if((lhs).size() == (rhs.size())) \
    { \
        bool anyNotEqual = false; \
        for (size_t i = 0; (i < (lhs).size()) && !anyNotEqual; i++) \
        { \
            anyNotEqual = ((lhs)[i] != (rhs)[i]); \
        } \
        __POTHOS_TEST_ASSERT( \
            "assert not equal " + Pothos::TestingBase::current().toString(lhs) + \
            " != " + Pothos::TestingBase::current().toString(rhs), anyNotEqual); \
    } \
}

//! Test two vectors for nonequality
#define POTHOS_TEST_NOT_CLOSEV(lhs, rhs, tol) \
{ \
    if((lhs).size() == (rhs.size())) \
    { \
        bool anyNotEqual = false; \
        for (size_t i = 0; (i < (lhs).size()) && !anyNotEqual; i++) \
        { \
            anyNotEqual = (std::abs((lhs)[i] - (rhs)[i]) <= (tol)); \
        } \
        __POTHOS_TEST_ASSERT( \
            "assert not close " + Pothos::TestingBase::current().toString(lhs) + \
            " !~= " + Pothos::TestingBase::current().toString(rhs), anyNotEqual); \
    } \
}

//
// Array comparisons
//

//! Test two arrays for equality
#define POTHOS_TEST_EQUALA(lhs, rhs, size) \
{ \
    for (size_t i = 0; i < (size); i++) \
    { \
        __POTHOS_TEST_ASSERT( \
            "index " + Pothos::TestingBase::current().toString(i) + \
            " asserts " + Pothos::TestingBase::current().toString((lhs)[i]) + \
            " == " + Pothos::TestingBase::current().toString((rhs)[i]), (lhs)[i] == (rhs)[i]); \
    } \
}

//! Test two arrays for equality within tolerance
#define POTHOS_TEST_CLOSEA(lhs, rhs, tol, size) \
{ \
    for (size_t i = 0; i < (size); i++) \
    { \
        __POTHOS_TEST_ASSERT( \
            "index " + Pothos::TestingBase::current().toString(i) + \
            " asserts " + Pothos::TestingBase::current().toString((lhs)[i]) + \
            " ~= " + Pothos::TestingBase::current().toString((rhs)[i]), (std::abs((lhs)[i] - (rhs)[i]) <= (tol))); \
    } \
}

//! Test two arrays for nonequality
#define POTHOS_TEST_NOT_EQUALA(lhs, rhs, size) \
{ \
    bool anyNotEqual = false; \
    for (size_t i = 0; (i < (size)) && !anyNotEqual; i++) \
    { \
        anyNotEqual = ((lhs) != (rhs)); \
    } \
    __POTHOS_TEST_ASSERT( \
        "assert arrays not equal ", anyNotEqual); \
}

//! Test two arrays for nonequality within tolerance
#define POTHOS_TEST_NOT_CLOSEA(lhs, rhs, tol, size) \
{ \
    bool anyNotEqual = false; \
    for (size_t i = 0; (i < (size)) && !anyNotEqual; i++) \
    { \
        anyNotEqual = (std::abs((lhs)[i] - (rhs)[i]) <= (tol)); \
    } \
    __POTHOS_TEST_ASSERT( \
        "assert arrays not close ", anyNotEqual); \
}

//
// Exception tests
//

//! Test that a statement throws a particular exception
#define POTHOS_TEST_THROWS(statement, expectedException) \
{ \
    Pothos::TestingBase::current().report(#statement, #statement, "", __LINE__, __FILE__); \
    __POTHOS_TEST_STATEMENT(#statement " must throw " #expectedException, \
        try{statement;throw std::string("statement \"" #statement "\" did not throw");} \
        catch(const expectedException &){}); \
}

namespace Pothos {

struct POTHOS_API TestingBase
{
    TestingBase(void);
    virtual ~TestingBase(void);
    static TestingBase &current(void);
    void runTests();
    virtual void runTestsImpl() = 0;
    void report(
        const std::string &message,
        const std::string &statement,
        const std::string &error,
        const int line,
        const std::string &file);
    template <typename T>
    std::string toString(const T &v)
    {
        return Pothos::Object(v).toString();
    }
};

} //namespace Pothos
