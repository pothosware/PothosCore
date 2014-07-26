//
// Remote/Client.hpp
//
// Remote access proxy client interface.
//
// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0
//

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Util/RefHolder.hpp>
#include <Pothos/Proxy/Environment.hpp>
#include <memory>
#include <string>
#include <iosfwd>

namespace Pothos {

/*!
 * A remote client is a handle for a client socket.
 * The socket can be interacted with through iostream.
 */
class POTHOS_API RemoteClient : public Util::RefHolder
{
public:

    //! Make an empty handle
    RemoteClient(void);

    /*!
     * Make a client handle to interact with a remote server.
     * A unspecified port means use the default locator port.
     * URI format: tcp://resolvable_hostname:optional_port
     * \param uri a formatted string which specifies a server
     * \param timeoutUs the timeout to connect in microseconds
     */
    RemoteClient(const std::string &uri, const long timeoutUs = 100000);

    //! Get the connection's URI
    const std::string &getUri(void) const;

    //! Is this remote client connected?
    pothos_explicit operator bool(void) const;

    /*!
     * Get the iostream to interact with the client handle.
     * This iostream can be passed to makeEnvironment to create a remote proxy.
     */
    std::iostream &getIoStream(void) const;

    /*!
     * Create a proxy environment that is interfaced through this remote client object.
     */
    ProxyEnvironment::Sptr makeEnvironment(const std::string &name, const ProxyEnvironmentArgs &args = ProxyEnvironmentArgs());

    /*!
     * Create a proxy environment that is interfaced over an iostream.
     * This allows for remote proxies that talk over pipes and sockets.
     */
    static ProxyEnvironment::Sptr makeEnvironment(std::istream &is, std::ostream &os,
        const std::string &name, const ProxyEnvironmentArgs &args = ProxyEnvironmentArgs());

    /*!
     * Create a proxy environment that is interfaced over an iostream.
     * This allows for remote proxies that talk over pipes and sockets.
     */
    static ProxyEnvironment::Sptr makeEnvironment(std::iostream &io,
        const std::string &name, const ProxyEnvironmentArgs &args = ProxyEnvironmentArgs());

private:
    struct Impl;
    std::shared_ptr<Impl> _impl;
};

} //namespace Pothos
