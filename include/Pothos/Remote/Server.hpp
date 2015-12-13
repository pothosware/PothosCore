///
/// \file Remote/Server.hpp
///
/// Remote access proxy server interface.
///
/// \copyright
/// Copyright (c) 2013-2015 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Util/RefHolder.hpp>
#include <memory>
#include <string>

namespace Pothos {

/*!
 * A remote server is a handle for an executing server process.
 * When all copies of the handle destruct, the server process will be terminated.
 */
class POTHOS_API RemoteServer : public Util::RefHolder
{
public:

    //! Make an empty handle
    RemoteServer(void);

    /*!
     * Spawn a new process on this machine, that is running a proxy server on the given URI.
     * URI format: tcp://resolvable_hostname:optional_port
     * A host address of 0.0.0.0 or [::] will bind the server to all interfaces.
     * An unspecified port means that an available port will be automatically chosen.
     * \param uri a formatted string which tells the server what kind of service to run
     * \param closePipes true to close stdout/err pipes (keep open for syslog forwarding)
     * \return a handle that when deleted, will cause the server/process to exit
     */
    RemoteServer(const std::string &uri, const bool closePipes = true);

    //! Get the server's bind URI
    const std::string &getUri(void) const;

    //! Is this remove server active?
    pothos_explicit operator bool(void) const;

    /*!
     * The locator port is the default port for running and locating a remote server.
     * Servers running on this part are used to establish initial communication.
     * Further communication can continue on arbitrary ports selected by the OS.
     */
    static std::string getLocatorPort(void);

    //! Get the actual port that the server is running on
    std::string getActualPort(void) const;

    /*!
     * Start syslog forwarding to the given address.
     * Spawn threads to read the stdout/err pipes from
     * the server process and forward to a syslog channel.
     * Only use when the server is created with closePipes false.
     * \param addr the log destination in host:port format
     * \param source the repoted source of the log messages
     */
    void startSyslogForwarding(const std::string &addr, const std::string &source);

private:
    struct Impl;
    std::shared_ptr<Impl> _impl;
};

} //namespace Pothos
