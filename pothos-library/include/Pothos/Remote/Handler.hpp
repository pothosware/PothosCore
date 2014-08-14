//
// Remote/Handler.hpp
//
// Proxy server instance handler.
//
// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0
//

#pragma once
#include <Pothos/Config.hpp>
#include <string>
#include <iosfwd>

namespace Pothos {

/*!
 * A server handler runs the Proxy service over an iostream.
 */
class POTHOS_API RemoteHandler
{
public:

    //! Make a new handler
    RemoteHandler(void);

    //! Make a new handler given the peer address
    RemoteHandler(const std::string &peerAddr);

    /*!
     * Run a handler for a remote proxy that is interfaced over an iostream.
     * This call blocks until the client's remote environment session destructs.
     */
    void runHandler(std::istream &is, std::ostream &os);

    /*!
     * Run a handler for a remote proxy that is interfaced over an iostream.
     * This call blocks until the client's remote environment session destructs.
     */
    void runHandler(std::iostream &io);

    /*!
     * Run the handler for a single request/response transaction.
     * \return done true when the client shuts-down the environment
     */
    bool runHandlerOnce(std::istream &is, std::ostream &os);

private:
    const std::string _peerAddr;
};

} //namespace Pothos
