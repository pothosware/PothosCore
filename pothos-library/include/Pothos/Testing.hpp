///
/// \file Testing.hpp
///
/// Macros for creating self-test plugins.
/// Self tests are installed into the registry and executed at runtime.
/// The test macros are loosely based on the Boost unit test suite.
///
/// \copyright
/// Copyright (c) 2013-2014 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Plugin.hpp>
#include <Pothos/Callable.hpp>
#include <Pothos/Exception.hpp>
#include <sstream>
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
struct name : Pothos::TestingBase \
{ \
    void runTestsImpl(void); \
    void runTestsImpl_(void); \
}; \
pothos_static_block(name) \
{ \
    std::shared_ptr<Pothos::TestingBase> testObj(new name()); \
    Pothos::PluginRegistry::add(Pothos::Plugin( \
        Pothos::PluginPath(path).join(#name), Pothos::Object(testObj))); \
} \
void name::runTestsImpl(void) \
{ \
    POTHOS_TEST_CHECKPOINT(); \
    this->runTestsImpl_(); \
} \
void name::runTestsImpl_(void)

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

//! Test if statement is true
#define POTHOS_TEST_TRUE(statement) \
{ \
    __POTHOS_TEST_ASSERT(#statement, statement) \
}

//! Test if an equality operation is true
#define POTHOS_TEST_EQUAL(lhs, rhs) \
{ \
    __POTHOS_TEST_ASSERT(Pothos::TestingBase::current().toString(lhs) + \
        " == " + Pothos::TestingBase::current().toString(rhs), (lhs) == (rhs)); \
}

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
    ~TestingBase(void);
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
        std::stringstream ss;
        ss << v;
        return ss.str();
    }
};

} //namespace Pothos
