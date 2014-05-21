// Copyright (C) by Ashton Mason. See LICENSE.txt for licensing information.


#ifndef TESTFRAMEWORK_ITESTSUITE_H
#define TESTFRAMEWORK_ITESTSUITE_H


#ifdef _MSC_VER
#pragma warning(push,0)
#pragma warning (disable:4530)  // C++ exception handler used, but unwind semantics are not enabled
#endif //_MSC_VER

#include <vector>
#include <string>

#ifdef _MSC_VER
#pragma warning(pop)
#endif //_MSC_VER


namespace TestFramework
{


/// Interface describing a suite of unit tests.
class ITestSuite
{
public:

    /// Defines a test suite name.
    typedef std::string TestSuiteName;

    /// Defines a test name.
    typedef std::string TestName;

    /// Defines a static test method.
    /// Static member functions count as static functions and can be used as tests.
    typedef void (*Test)();

    /// Defines a list of tests in a test suite.
    typedef std::vector<Test> TestList;

    /// Defines a list of test names.
    typedef std::vector<TestName> TestNameList;

    /// Defines a test error message.
    typedef std::string Error;

    /// Defines a list of test error messages.
    typedef std::vector<Error> ErrorList;

    /// Default constructor
    inline ITestSuite()
    {
    }

    /// Virtual destructor
    inline virtual ~ITestSuite()
    {
    }

    /// Runs all tests in the suite.
    /// \return True, if all the tests in the suite passed.
    virtual bool RunTests(const bool verbose) = 0;

    /// Gets the errors returned by the failed tests in the suite.
    /// \return A list of errors.
    virtual const ErrorList &GetErrors() = 0;

private:

    /// Disallowed copy constructor. TestSuite objects can't be copied.
    ITestSuite(const ITestSuite &other);
    /// Disallowed assignment operator. ITestSuite objects can't be assigned.
    ITestSuite &operator=(const ITestSuite &other);
};


} // namespace TestFramework


#endif // TESTFRAMEWORK_ITESTSUITE_H

