// Copyright (c) 2020 Nicholas Corgan
// SPDX-License-Identifier: BSL-1.0

#include <Poco/Platform.h>

#include <Pothos/Testing.hpp>
#include <Pothos/Util/ExceptionForErrorCode.hpp>

#include <cstring>
#include <iostream>
#include <string>

POTHOS_TEST_BLOCK("/util/tests", test_errno_return_code)
{
    constexpr int errCode = EINVAL;

    // Don't use testing macro because we need to check the error message.
    try
    {
        throw Pothos::Util::ErrnoException<Pothos::SystemException>(errCode);
        __POTHOS_TEST_STATEMENT("This should have thrown", (void)false);
    }
    catch (const Pothos::SystemException& ex)
    {
        const auto errorMessage = ex.displayText();
        POTHOS_TEST_NOT_EQUAL(std::string::npos, errorMessage.find(std::to_string(errCode)));
    }
    catch (...)
    {
        __POTHOS_TEST_STATEMENT("Did not throw Pothos::SystemException", (void)false);
    }
}

POTHOS_TEST_BLOCK("/util/tests", test_errno)
{
    constexpr int errCode = EOVERFLOW;

    // Don't use testing macro because we need to check the error message.
    try
    {
        errno = errCode;
        throw Pothos::Util::ErrnoException<Pothos::SystemException>();
        __POTHOS_TEST_STATEMENT("This should have thrown", (void)false);
    }
    catch(const Pothos::SystemException& ex)
    {
        const auto errorMessage = ex.displayText();
        POTHOS_TEST_NOT_EQUAL(std::string::npos, errorMessage.find(std::to_string(errCode)));
    }
    catch (...)
    {
        __POTHOS_TEST_STATEMENT("Did not throw Pothos::SystemException", (void)false);
    }
}

#ifdef POCO_OS_FAMILY_WINDOWS

POTHOS_TEST_BLOCK("/util/tests", test_winerror_return_code)
{
    constexpr int errCode = ERROR_ACCESS_DENIED;
    const auto expectedErrorMessage = "Access is denied.";

    // Don't use testing macro because we need to check the error message.
    try
    {
        throw Pothos::Util::WinErrorException<Pothos::SystemException>(errCode);
        __POTHOS_TEST_STATEMENT("This should have thrown", (void)false);
    }
    catch (const Pothos::SystemException& ex)
    {
        const auto errorMessage = ex.message();
        POTHOS_TEST_NOT_EQUAL(std::string::npos, errorMessage.find(std::to_string(errCode)));
        POTHOS_TEST_NOT_EQUAL(std::string::npos, errorMessage.find(expectedErrorMessage));
    }
}

POTHOS_TEST_BLOCK("/util/tests", test_last_winerror)
{
    constexpr int errCode = ERROR_FILE_TOO_LARGE;
    const auto expectedErrorMessage = "The file size exceeds the limit allowed and cannot be saved.";

    // Don't use testing macro because we need to check the error message.
    try
    {
        ::SetLastError(errCode);
        throw Pothos::Util::WinErrorException<Pothos::SystemException>();
        __POTHOS_TEST_STATEMENT("This should have thrown", (void)false);
    }
    catch (const Pothos::SystemException& ex)
    {
        const auto errorMessage = ex.message();
        POTHOS_TEST_NOT_EQUAL(std::string::npos, errorMessage.find(std::to_string(errCode)));
        POTHOS_TEST_NOT_EQUAL(std::string::npos, errorMessage.find(expectedErrorMessage));
    }
}

#endif

//
// Basic std::error_category subclass to test passing in arbitrary categories. Note that any
// std::error_category passed in must be a singleton.
//
enum class TestErrorCode: int
{
    OK = 0,
    ACCESS_DENIED,
    INVALID_ARGUMENT,
    GENERIC_ERROR
};
class TestErrorCategoryImpl : public std::error_category
{
public:

    static const TestErrorCategoryImpl& instance()
    {
        static TestErrorCategoryImpl ec;
        return ec;
    }

    const char* name() const noexcept override { return "test"; }

    std::error_condition default_error_condition(int code) const noexcept override
    {
        switch (static_cast<TestErrorCode>(code))
        {
            case TestErrorCode::OK: return std::error_condition(0, std::generic_category());
            case TestErrorCode::ACCESS_DENIED: return std::make_error_condition(std::errc::permission_denied);
            case TestErrorCode::INVALID_ARGUMENT: return std::make_error_condition(std::errc::invalid_argument);
            default: return std::make_error_condition(std::errc::not_supported);
        }
    }

    bool equivalent(int code, const std::error_condition& ec) const noexcept override
    {
        return (ec.value() == code);
    }

    std::string message(int code) const override
    {
        switch (static_cast<TestErrorCode>(code))
        {
        case TestErrorCode::OK: return "OK (test)";
        case TestErrorCode::ACCESS_DENIED: return "Access denied (test)";
        case TestErrorCode::INVALID_ARGUMENT: return "Invalid argument (test)";
        default: return "Generic error (test)";
        }
    }

private:
    TestErrorCategoryImpl() {}
    virtual ~TestErrorCategoryImpl() {}
};
static const std::error_category& TestErrorCategory()
{
    return TestErrorCategoryImpl::instance();
}

POTHOS_TEST_BLOCK("/util/tests", test_custom_error_category)
{
    constexpr auto errCode = static_cast<int>(TestErrorCode::ACCESS_DENIED);
    const auto expectedErrorMessage = "Access denied (test)";

    // Don't use testing macro because we need to check the error message.
    try
    {
        throw Pothos::Util::ExceptionForErrorCode<Pothos::ApplicationException>(
                  errCode,
                  TestErrorCategory());
        __POTHOS_TEST_STATEMENT("This should have thrown", (void)false);
    }
    catch (const Pothos::ApplicationException& ex)
    {
        const auto errorMessage = ex.message();
        POTHOS_TEST_NOT_EQUAL(std::string::npos, errorMessage.find(std::to_string(errCode)));
        POTHOS_TEST_NOT_EQUAL(std::string::npos, errorMessage.find(expectedErrorMessage));
    }
}