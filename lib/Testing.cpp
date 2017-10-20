// Copyright (c) 2013-2017 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Poco/Logger.h>
#include <Poco/Path.h>
#include <Poco/Format.h>
#include <mutex>
#include <vector>
#include <cassert>

static std::mutex &getTestMutex(void)
{
    static std::mutex mutex;
    return mutex;
}

Pothos::TestingBase::TestingBase(void)
{
    return;
}

Pothos::TestingBase::~TestingBase(void)
{
    return;
}

/*!
 * The special report exception thrown by report on errors
 * so that the test handler will know its an intentional exit.
 */
struct TestingReportError : Pothos::Exception
{
    TestingReportError(const std::string &what):
        Pothos::Exception(what)
    {
        return;
    }
};

static Pothos::TestingBase *testInProgress = nullptr;
static int lastLine = 0;
static std::string lastFile;

Pothos::TestingBase &Pothos::TestingBase::current(void)
{
    assert(testInProgress != nullptr);
    return *testInProgress;
}

void Pothos::TestingBase::runTests(void)
{
    std::lock_guard<std::mutex> lock(getTestMutex());
    testInProgress = this;
    POTHOS_EXCEPTION_TRY
    {
        lastLine = -1;
        lastFile = "";
        this->runTestsImpl();
    }
    POTHOS_EXCEPTION_CATCH(const Exception &ex)
    {
        try
        {
            const TestingReportError &reportError = dynamic_cast<const TestingReportError &>(ex);
            throw reportError;
        }
        catch (const std::bad_cast &)
        {
            //unknown, report a new error
        }
        this->report("Unexpected error after last checkpoint", "runTests()", ex.displayText(), lastLine, lastFile);
    }
    testInProgress = nullptr;
}

void Pothos::TestingBase::report(
    const std::string &message,
    const std::string &/*statement*/,
    const std::string &error,
    const int line,
    const std::string &file)
{
    //cache line and file to report possible location of unexpected errors
    lastLine = line;
    lastFile = file;

    //format an informational message
    std::string testMessage = Poco::format(
        "%s:%d\n  %s", Poco::Path(file).getFileName(), line, message);
    if (error.empty())
    {
        poco_debug(Poco::Logger::get("Pothos.Testing"), testMessage);
    }
    else
    {
        testMessage += "\n  " + error;
        poco_debug(Poco::Logger::get("Pothos.Testing"), testMessage);
        throw TestingReportError("Pothos::Testing " + testMessage);
    }
}
