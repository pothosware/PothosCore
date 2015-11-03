///
/// \file System/Logger.hpp
///
/// API calls for logger control.
///
/// \copyright
/// Copyright (c) 2014-2015 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Config.hpp>
#include <string>

namespace Pothos {
namespace System {

/*!
 * Logger class contains static methods to deal with log configuration.
 */
class POTHOS_API Logger
{
public:
    /*!
     * Start a listener for syslog messages from other processes.
     * The log messages will be forwarded to the default logger.
     * \return the port that this process is listening
     */
    static std::string startSyslogListener(void);

    /*!
     * Stop the syslog listener service.
     */
    static void stopSyslogListener(void);

    /*!
     * Start syslog forwarding to the given address.
     * \param addr the log destination in host:port format
     */
    static void startSyslogForwarding(const std::string &addr);

    /*!
     * Stop all syslog forwarding setup by startSyslogForwarding().
     */
    static void stopSyslogForwarding(void);

    /*!
     * Redirect standard IO to the logging facility.
     * This allows the user to capture standard IO debug messages
     * from subprocesses and servers that would otherwise be lost.
     * Because the default logging output directs to standard IO,
     * it is recommended to first change the default logging channel.
     * For example, startSyslogForwarding() will direct logs to UDP.
     * \param source the repoted source of the forwarded log messages
     */
    static void forwardStdIoToLogging(const std::string &source);

    /*!
     * Setup default logging for this process.
     * The default logger will log all non-informational logs to the console.
     * The logger can be configured with the following environment vars:
     * POTHOS_LOG_LEVEL - what level to display logs (default notice)
     * POTHOS_LOG_CHANNEL - how to display the logs (default color)
     * POTHOS_LOG_FILE - log file path if POTHOS_LOG_CHANNEL=file specified
     */
    static void setupDefaultLogging(void);

private:
    //! private constructor: we dont make Logger instances
    Logger(void){}
};

} //namespace System
} //namespace Pothos
