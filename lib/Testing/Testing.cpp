// Copyright (c) 2013-2017 Josh Blum
//                    2020 Nicholas Corgan
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

//
// Not all macros are used within PothosCore's testing, so we'll add
// specific tests for each one to make sure they compile and work as
// expected.
//

POTHOS_TEST_BLOCK("/testing/tests", test_asserts)
{
    std::vector<int> vec;
    POTHOS_TEST_TRUE(vec.empty());

    vec.emplace_back(0);
    POTHOS_TEST_FALSE(vec.empty());
}

POTHOS_TEST_BLOCK("/testing/tests", test_scalar_comparison)
{
    constexpr int intVal1 = 0;
    constexpr int intVal2 = 0;
    constexpr int intVal3 = 1;

    POTHOS_TEST_EQUAL(intVal1, intVal2);
    POTHOS_TEST_NOT_EQUAL(intVal1, intVal3);
    POTHOS_TEST_LE(intVal1, intVal2);
    POTHOS_TEST_LT(intVal1, intVal3);
    POTHOS_TEST_GE(intVal2, intVal1);
    POTHOS_TEST_GT(intVal3, intVal1);

    constexpr float epsilon = 1e-3;
    constexpr float floatVal1 = 0.0;
    constexpr float floatVal2 = (floatVal1 + (epsilon / 2.0f));
    constexpr float floatVal3 = (floatVal1 - (epsilon * 2.0f));

    POTHOS_TEST_CLOSE(floatVal1, floatVal2, epsilon);
    POTHOS_TEST_NOT_CLOSE(floatVal1, floatVal3, epsilon);
}

POTHOS_TEST_BLOCK("/testing/tests", test_vector_comparison)
{
    const std::vector<int> intVec1 = {0,1,2,3,4};
    const std::vector<int> intVec2 = {0,1,2,3,4};
    const std::vector<int> intVec3 = {0,1,2,3,4,5};
    const std::vector<int> intVec4 = {0,1,2,3,5};

    POTHOS_TEST_EQUALV(intVec1, intVec2);
    POTHOS_TEST_NOT_EQUALV(intVec1, intVec3);
    POTHOS_TEST_NOT_EQUALV(intVec1, intVec4);

    constexpr float epsilon = 1e-3;
    const std::vector<float> floatVec1 = {0,1,2,3,4};
    const std::vector<float> floatVec2 = {0,1,(2+(epsilon/2.0f)),3,4};
    const std::vector<float> floatVec3 = {0,1,2,3,4,5};
    const std::vector<float> floatVec4 = {0,1,2,3,(4+(epsilon*2.0f))};

    POTHOS_TEST_CLOSEV(floatVec1, floatVec2, epsilon);
    POTHOS_TEST_NOT_CLOSEV(floatVec1, floatVec3, epsilon);
    POTHOS_TEST_NOT_CLOSEV(floatVec1, floatVec4, epsilon);
}

POTHOS_TEST_BLOCK("/testing/tests", test_array_comparison)
{
    constexpr size_t arrSize = 5;

    int intArr1[arrSize] = {0,1,2,3,4};
    int intArr2[arrSize] = {0,1,2,3,4};
    int intArr3[arrSize] = {0,1,2,3,5};

    POTHOS_TEST_EQUALA(intArr1, intArr2, arrSize);
    POTHOS_TEST_NOT_EQUALA(intArr1, intArr3, arrSize);

    constexpr float epsilon = 1e-3;
    float floatArr1[arrSize] = {0,1,2,3,4};
    float floatArr2[arrSize] = {0,1,(2+(epsilon/2.0f)),3,4};
    float floatArr3[arrSize] = {0,1,2,3,(4+(epsilon*2.0f))};

    POTHOS_TEST_CLOSEA(floatArr1, floatArr2, epsilon, arrSize);
    POTHOS_TEST_NOT_CLOSEA(floatArr1, floatArr3, epsilon, arrSize);
}
