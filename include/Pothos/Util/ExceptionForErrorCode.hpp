///
/// \file Util/ErrorCodeHandling.hpp
///
/// Mapping sets of error codes to C++ exceptions
///
/// \copyright
/// Copyright (c) 2020 Nicholas Corgan
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Exception.hpp>

#include <cerrno>
#include <cmath>
#include <system_error>

#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>
#endif

namespace Pothos {
namespace Util {

    /*!
     * An exception corresponding to a specific error code, passing in a
     * std::error_category instance to access the associated error message. Error
     * codes for OS functions can be queried using std::generic_category() or
     * std::system_category(). Alternatively, a custom std::error_category subclass
     * can be passed in to convert arbitrary error codes to exceptions.
     */
    template <typename ExceptionType = Pothos::Exception>
    class ExceptionForErrorCode : public ExceptionType
    {
    public:
        /*!
         * \param errorCode The error code to convert into an exception
         * \param category A std::error_category mapping the error code to an error string
         */
        ExceptionForErrorCode(int errorCode, const std::error_category& category) :
            ExceptionType("Error code " + std::to_string(errorCode) + ": " + category.message(errorCode))
        {}

        virtual ~ExceptionForErrorCode() = default;
    };

    /*!
     * An exception corresponding to the set of POSIX error codes supported on the given
     * platform.
     *
     * On POSIX systems, std::generic_category() and std::system_category()
     * return the same set of errors. On Windows systems, std::generic_category()
     * returns errno's error set. Given this, std::generic_category() is used.
     */
    template <typename ExceptionType = Pothos::Exception>
    class ErrnoException : public ExceptionForErrorCode<ExceptionType>
    {
    public:
        /*!
         * Base this exception on the error code currently stored in the global errno.
         *
         * Note that this accounts for some functions setting errno to the negative of
         * the actual code.
         */
        ErrnoException() : ExceptionForErrorCode<ExceptionType>(std::abs(errno), std::generic_category())
        {}

        /*!
         * Base this exception on the given POSIX error code.
         *
         * Note that this accounts for some functions returning the negative of the actual code.
         *
         * \param errorCode The POSIX error code to convert into an exception
         */
        ErrnoException(int errorCode) : ExceptionForErrorCode<ExceptionType>(std::abs(errorCode), std::generic_category())
        {}

        virtual ~ErrnoException() = default;
    };

#if defined(_WIN32) || defined(_WIN64)
    /*!
     * An exception corresponding to the set of Windows error codes supported on the given
     * platform. Uses std::system_category(), as all MSVC implementations store Windows
     * error code strings in this category.
     */
    template <typename ExceptionType = Pothos::Exception>
    class WinErrorException : public ExceptionForErrorCode<ExceptionType>
    {
    public:
        /*!
         * Base this exception on the error code returned by GetLastError().
         */
        WinErrorException() : ExceptionForErrorCode<ExceptionType>(::GetLastError(), std::system_category())
        {}

        /*!
         * Base this exception on the given Windows error code.
         *
         * \param errorCode The Windows error code to convert into an exception
         */
        WinErrorException(int errorCode) : ExceptionForErrorCode<ExceptionType>(errorCode, std::system_category())
        {}

        virtual ~WinErrorException() = default;
    };
#endif
} //namespace Util
} //namespace Pothos