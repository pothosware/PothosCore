//
// Remote/Server.hpp
//
// Remote access proxy server interface.
//
// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0
//

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Util/RefHolder.hpp>
#include <memory>
#include <string>
#include <iosfwd>

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
     * A host address of 0.0.0.0 will bind the server to all interfaces.
     * An unspecified port means that an available port will be automatically chosen.
     * \param uri a formatted string which tells the server what kind of service to run
     * \return a handle that when deleted, will cause the server/process to exit
     */
    RemoteServer(const std::string &uri);

    //! Is this remote server object null (true when empty)
    bool null(void) const;

    /*!
     * The locator port is the default port for running and locating a remote server.
     * Servers running on this part are used to establish initial communication.
     * Further communication can continue on arbitrary ports selected by the OS.
     */
    static std::string getLocatorPort(void);

    //! Get the actual port that the server is running on
    std::string getActualPort(void) const;

    /*!
     * Run a handler for a remote proxy that is interfaced over an iostream.
     * This call blocks until the client's remote environment session destructs.
     */
    static void runHandler(std::istream &is, std::ostream &os);

    /*!
     * Run a handler for a remote proxy that is interfaced over an iostream.
     * This call blocks until the client's remote environment session destructs.
     */
    static void runHandler(std::iostream &io);

private:
    struct Impl;
    std::shared_ptr<Impl> _impl;
};

} //namespace Pothos
