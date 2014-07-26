// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Remote.hpp>
#include <Pothos/Remote/Exception.hpp>
#include <Poco/Net/StreamSocket.h>
#include <Poco/Net/SocketStream.h>
#include <Poco/URI.h>
#include <cassert>

struct Pothos::RemoteClient::Impl
{
    Impl(const std::string &uriStr, const long timeoutUs):
        socketStream(clientSocket),
        uriStr(uriStr)
    {
        POTHOS_EXCEPTION_TRY
        {
            Poco::URI uri(uriStr);
            if (uri.getScheme() != "tcp") throw InvalidArgumentException("unsupported URI scheme");
        }
        POTHOS_EXCEPTION_CATCH(const Exception &ex)
        {
            throw RemoteClientError("Pothos::RemoteClient("+uriStr+")", ex);
        }

        try
        {
            Poco::URI uri(uriStr);
            const std::string host = uri.getHost();
            std::string port = std::to_string(uri.getPort());
            if (port == "0") port = RemoteServer::getLocatorPort();
            Poco::Net::SocketAddress sa(host, port);
            clientSocket.connect(sa, Poco::Timespan(0, timeoutUs));
        }
        catch (const Poco::Exception &ex)
        {
            throw RemoteClientError("Pothos::RemoteClient("+uriStr+")", ex.displayText());
        }

    }
    Poco::Net::StreamSocket clientSocket;
    Poco::Net::SocketStream socketStream;
    const std::string uriStr;
};

Pothos::RemoteClient::RemoteClient(void)
{
    assert(not *this);
}

Pothos::RemoteClient::RemoteClient(const std::string &uri, const long timeoutUs):
    _impl(new Impl(uri, timeoutUs))
{
    return;
}

const std::string &Pothos::RemoteClient::getUri(void) const
{
    assert(_impl);
    return _impl->uriStr;
}

Pothos::RemoteClient::operator bool(void) const
{
    return bool(_impl);
}

std::iostream &Pothos::RemoteClient::getIoStream(void) const
{
    assert(_impl);
    return _impl->socketStream;
}

Pothos::ProxyEnvironment::Sptr Pothos::RemoteClient::makeEnvironment(const std::string &name, const ProxyEnvironmentArgs &args)
{
    assert(_impl);
    auto env = RemoteClient::makeEnvironment(this->getIoStream(), name, args);
    env->holdRef(Object(*this));
    return env;
}

#include <Pothos/Managed.hpp>

static auto managedRemoteClient = Pothos::ManagedClass()
    .registerConstructor<Pothos::RemoteClient>()
    .registerConstructor<Pothos::RemoteClient, std::string>()
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::RemoteClient, getIoStream))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::RemoteClient, makeEnvironment))
    .registerStaticMethod<std::iostream &>(POTHOS_FCN_TUPLE(Pothos::RemoteClient, makeEnvironment))
    .registerStaticMethod<std::istream &, std::ostream &>(POTHOS_FCN_TUPLE(Pothos::RemoteClient, makeEnvironment))
    .commit("Pothos/RemoteClient");
