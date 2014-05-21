// Copyright (C) by Ashton Mason. See LICENSE.txt for licensing information.


#ifndef TESTFRAMEWORK_TESTMANAGER_H
#define TESTFRAMEWORK_TESTMANAGER_H


#ifdef _MSC_VER
#pragma warning(push,0)
#pragma warning (disable:4530)  // C++ exception handler used, but unwind semantics are not enabled
#endif //_MSC_VER

#include <vector>

#ifdef _MSC_VER
#pragma warning(pop)
#endif //_MSC_VER

#include "ITestSuite.h"


namespace TestFramework
{


/// Singleton manager/factory class that manages a collection of unit tests suites.
class TestManager
{
public:

    /// Defines a list of pointers to test suites.
    typedef std::vector<ITestSuite *> TestSuiteList;

    /// Defines a test error message.
    typedef ITestSuite::Error Error;

    /// Defines a list of test error messages.
    typedef ITestSuite::ErrorList ErrorList;

    /// Destructor
    inline ~TestManager()
    {
    }

    /// Static method that returns a pointer to the single instance of the TestManager singleton.
    inline static TestManager *Instance()
    {
        static TestManager sInstance;
        return &sInstance;
    }

    /// Registers a test suite with the test suite manager.
    /// \param testSuite The test suite to be registered, which must implement ITestSuite.
    inline void Register(ITestSuite *const testSuite)
    {
        mTestSuites.push_back(testSuite);
    }

    /// Registers a test suite with the test suite manager.
    /// \return True if the test suites all passed, otherwise false.
    inline bool RunTests(const bool verbose)
    {
        mErrors.clear();

        bool passedAllSuites = true;

        for (TestSuiteList::const_iterator it = mTestSuites.begin(); it != mTestSuites.end(); ++it)
        {
            ITestSuite *const testSuite = (*it);
            const bool passedSuite = testSuite->RunTests(verbose);

            if (!passedSuite)
            {
                // Collect the errors from the test suite.
                const ErrorList &suiteErrors(testSuite->GetErrors());
                mErrors.insert(mErrors.end(), suiteErrors.begin(), suiteErrors.end());
                passedAllSuites = false;
            }
        }

        return passedAllSuites;
    }

    /// Gets the errors returned by the failed tests in all registered test suites.
    /// \return A list of errors.
    inline const ErrorList &GetErrors()
    {
        return mErrors;
    }

private:

    /// Private default constructor. This is a singleton class and can't be constructed directly.
    inline TestManager()
    {
    }

    /// Disallowed copy constructor. TestManager objects can't be copied.
    TestManager(const TestManager &other);
    /// Disallowed assignment operator. TestManager objects can't be assigned.
    TestManager &operator=(const TestManager &other);

    TestSuiteList mTestSuites;          ///< List of test suites managed by the manager.
    ErrorList mErrors;                  ///< List of error messages returned by failed tests.
};


} // namespace TestFramework


#endif // TESTFRAMEWORK_TESTMANAGER_H

