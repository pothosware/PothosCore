// Copyright (C) by Ashton Mason. See LICENSE.txt for licensing information.


#ifndef TESTFRAMEWORK_TESTSUITE_H
#define TESTFRAMEWORK_TESTSUITE_H


#include <stdio.h>

#include "ITestSuite.h"
#include "TestManager.h"
#include "TestException.h"


/// Macro that registers a test suite.
#define TESTFRAMEWORK_REGISTER_TESTSUITE(testSuiteName) RegisterTestSuite(#testSuiteName)

/// Macro that registers a test method with a test suite.
#define TESTFRAMEWORK_REGISTER_TEST(testName) RegisterTest(testName, #testName)


namespace TestFramework
{


/// Abstract base class containing a suite of unit tests.
/// Users should derive their own test suites from this base class.
class TestSuite : public ITestSuite
{

public:

    /// Runs all tests in the suite.
    /// \return True, if all the tests in the suite passed.
    inline virtual bool RunTests(const bool verbose)
    {
        mErrors.clear();
        bool passedAllTests = true;

        TestList::const_iterator testIt = mTests.begin();
        TestNameList::const_iterator nameIt = mTestNames.begin();

        while (testIt != mTests.end())
        {
            const TestName name = (*nameIt);
            const Test test = (*testIt);

            if (verbose)
            {
                printf("Test: %s::%s\n", mTestSuiteName.c_str(), name.c_str());
                fflush(stdout);
            }

            // Tests throw TestExceptions on failure.
            try
            {
                test();
            }
            catch (TestException *e)
            {
                const Error error("FAILED: " + mTestSuiteName + "::" + name + ": " + e->GetMessage());
                mErrors.push_back(error);
                passedAllTests = false;

                printf("%s\n", error.c_str());
                fflush(stdout);
            }
            catch (...)
            {
                const Error error("FAILED: " + mTestSuiteName + "::" + name + ": Unknown exception");
                mErrors.push_back(error);
                passedAllTests = false;

                printf("%s\n", error.c_str());
                fflush(stdout);
            }

            ++testIt;
            ++nameIt;
        }

        return passedAllTests;
    }

protected:

    /// Default constructor
    inline TestSuite()
    {
        // Auto-register ourselves with the test manager.
        TestManager::Instance()->Register(this);
    }

    /// Virtual destructor
    inline virtual ~TestSuite()
    {
    }

    /// Registers a test suite.
    /// \param name An identifying name for the test suite.
    inline void RegisterTestSuite(const TestSuiteName &name)
    {
        mTestSuiteName = name;
    }

    /// Registers a test within the suite.
    /// \param test The test method to be registered.
    /// \param name An identifying name for the test.
    inline void RegisterTest(Test test, const TestName &name)
    {
        mTests.push_back(test);
        mTestNames.push_back(name);
    }

    /// Gets the errors returned by the failed tests in the suite.
    /// \return A list of errors.
    inline virtual const ErrorList &GetErrors()
    {
        return mErrors;
    }

    /// Checks that a given condition is true.
    /// \param condition The condition to be tested.
    /// \param error An error message to be thrown if the condition is untrue.
    /// Throws a TestException if the condition is false.
    inline static void Check(const bool condition, const Error &error)
    {
        if (!(condition))
        {
            throw new TestException(error);
        }
    }

private:

    /// Disallowed copy constructor. TestSuite objects can't be copied.
    TestSuite(const TestSuite &other);
    /// Disallowed assignment operator. TestSuite objects can't be assigned.
    TestSuite &operator=(const TestSuite &other);

    TestSuiteName mTestSuiteName;   ///< Name of this test suite.
    TestList mTests;                ///< List of tests in this suite.
    TestNameList mTestNames;        ///< List of names of the tests in this suite.
    ErrorList mErrors;              ///< List of errors from tests in this suite.
};


} // namespace TestFramework


#endif // TESTFRAMEWORK_TESTSUITE_H

