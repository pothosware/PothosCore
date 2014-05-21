//
// System/Logger.hpp
//
// API calls for logger control.
//
// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0
//

#pragma once
#include <Pothos/Config.hpp>
#include <string>

namespace Pothos {
namespace System {

    /*!
     * Start a listener for syslog messages from other processes.
     * The log messages will be forwarded to the default logger.
     * \return the port that this process is listening
     */
    POTHOS_API std::string startSyslogListener(void);

    /*!
     * Start syslog forwarding to the given address.
     * \param addr the log destination in host:port format
     */
    POTHOS_API void startSyslogForwarding(const std::string &addr);

    /*!
     * Setup default logging for this process.
     * The default logger will log all non-informational logs to the console.
     * The logger can be configured with the following environment vars:
     * POTHOS_LOG_LEVEL - what level to display logs (default notice)
     * POTHOS_LOG_CHANNEL - how to display the logs (default color)
     * POTHOS_LOG_FILE - log file path if POTHOS_LOG_CHANNEL=file specified
     */
    POTHOS_API void setupDefaultLogging(void);

} //namespace System
} //namespace Pothos
