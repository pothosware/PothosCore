// Copyright (C) by Ashton Mason. See LICENSE.txt for licensing information.


#ifndef TESTFRAMEWORK_TESTEXCEPTION_H
#define TESTFRAMEWORK_TESTEXCEPTION_H


#ifdef _MSC_VER
#pragma warning(push,0)
#pragma warning (disable:4530)  // C++ exception handler used, but unwind semantics are not enabled
#endif //_MSC_VER

#include <string>

#ifdef _MSC_VER
#pragma warning(pop)
#endif //_MSC_VER


namespace TestFramework
{


/// Simple exception type for use in unit tests.
class TestException
{
public:

    /// Constructor.
    /// \param msg A string message associated with the exception.
    explicit TestException(const std::string &msg) : mMsg(msg)
    {
    }

    /// Virtual destructor.
    virtual ~TestException()
    {
    }

    const std::string &GetMessage()
    {
        return mMsg;
    }

private:

    /// Disallowed default constructor
    TestException() { }

    /// Disallowed copy constructor.
    TestException(const TestException &other);
    /// Disallowed assignment operator.
    TestException &operator=(const TestException &other);

    /// Message associated with the exception.
    std::string mMsg;
};


} // namespace TestFramework


#endif // TESTFRAMEWORK_TESTEXCEPTION_H

