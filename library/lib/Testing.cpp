// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Poco/Logger.h>
#include <Poco/Path.h>
#include <Poco/Format.h>
#include <mutex>
#include <Poco/SingletonHolder.h>
#include <vector>
#include <cassert>

static std::mutex &getTestMutex(void)
{
    static Poco::SingletonHolder<std::mutex> sh;
    return *sh.get();
}

Pothos::TestingBase::TestingBase(void)
{
    return;
}

Pothos::TestingBase::~TestingBase(void)
{
    return;
}

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
        "%s:%d -- %s", Poco::Path(file).getFileName(), line, message);
    if (error.empty())
    {
        poco_information(Poco::Logger::get("Pothos.Testing"), testMessage);
    }
    else
    {
        testMessage += "; Error: " + error;
        poco_information(Poco::Logger::get("Pothos.Testing"), testMessage);
        throw Exception("Pothos::Testing " + testMessage);
    }
}
