// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Remote.hpp>
#include <Poco/Net/StreamSocket.h>
#include <Poco/Net/SocketStream.h>
#include <Poco/URI.h>
#include <Poco/SingletonHolder.h>
#include <Poco/RWLock.h>
#include <future>
#include <mutex>
#include <map>
#include <cassert>

/***********************************************************************
 * lookupIpFromNodeId implementation
 **********************************************************************/
static Poco::RWLock &getLookupMutex(void)
{
    static Poco::SingletonHolder<Poco::RWLock> sh;
    return *sh.get();
}

static std::map<std::string, Poco::Net::IPAddress> &getNodeIdTable(void)
{
    static Poco::SingletonHolder<std::map<std::string, Poco::Net::IPAddress>> sh;
    return *sh.get();
}

std::string Pothos::RemoteClient::lookupIpFromNodeId(const std::string nodeId)
{
    Poco::RWLock::ScopedReadLock lock(getLookupMutex());
    auto it = getNodeIdTable().find(nodeId);
    if (it == getNodeIdTable().end()) return "";
    return it->second.toString();
}

static void updateNodeIdTable(const Pothos::ProxyEnvironment::Sptr &env, const Poco::Net::IPAddress &ipAddr)
{
    Poco::RWLock::ScopedWriteLock lock(getLookupMutex());
    getNodeIdTable()[env->getNodeId()] = ipAddr;
}

/***********************************************************************
 * lookupIpFromNodeId implementation
 **********************************************************************/
static std::mutex &getDNSLookupMutex(void)
{
    static Poco::SingletonHolder<std::mutex> sh;
    return *sh.get();
}

static std::map<std::string, std::shared_future<Poco::Net::IPAddress>> &getDNSFutures(void)
{
    static Poco::SingletonHolder<std::map<std::string, std::shared_future<Poco::Net::IPAddress>>> sh;
    return *sh.get();
}

//! The blocking DNS lookup routine -- perfomed by an async future
static Poco::Net::IPAddress dnsLookupBlocking(const std::string &host)
{
    return Poco::Net::SocketAddress(host, 0).host();
}

//! Perform a DNS lookup with a timeout - throws Poco::Exception
static Poco::Net::IPAddress dnsLookup(const std::string &host, const long timeoutUs)
{
    //create a future or get an existing one
    std::shared_future<Poco::Net::IPAddress> dnsFuture;
    {
        std::lock_guard<std::mutex> l(getDNSLookupMutex());
        if (getDNSFutures().count(host) == 0)
        {
            getDNSFutures()[host] = std::async(std::launch::async, &dnsLookupBlocking, host);
        }
        dnsFuture = getDNSFutures().at(host);
    }

    //wait with a timeout for the future to complete
    if (dnsFuture.wait_for(std::chrono::microseconds(timeoutUs)) != std::future_status::ready)
    {
        throw Poco::TimeoutException("DNS lookup for "+host);
    }

    //get the future, may throw exceptions from SocketAddress
    const auto ipAddr = dnsFuture.get();

    //remove the completed future from the map
    {
        std::lock_guard<std::mutex> l(getDNSLookupMutex());
        getDNSFutures().erase(host);
    }
    return ipAddr;
}

/***********************************************************************
 * RemoteClient implementation
 **********************************************************************/
struct Pothos::RemoteClient::Impl
{
    Impl(const std::string &uriStr, const long timeoutUs):
        socketStream(clientSocket),
        uriStr(uriStr)
    {
        //validate the URI
        POTHOS_EXCEPTION_TRY
        {
            Poco::URI uri(uriStr);
            if (uri.getScheme() != "tcp") throw InvalidArgumentException("unsupported URI scheme");
        }
        POTHOS_EXCEPTION_CATCH(const Exception &ex)
        {
            throw RemoteClientError("Pothos::RemoteClient("+uriStr+")", ex);
        }

        //extract port, for unspecified port -- use the default locator port
        Poco::URI uri(uriStr);
        auto port = uri.getPort();
        if (port == 0) port = std::stoi(RemoteServer::getLocatorPort());

        //perform the dns lookup
        try
        {
            this->sa = Poco::Net::SocketAddress(dnsLookup(uri.getHost(), timeoutUs), port);
        }
        catch (const Poco::Exception &ex)
        {
            throw RemoteClientError("Pothos::RemoteClient("+uriStr+")", ex.displayText());
        }

        //try to connect to the server
        try
        {
            clientSocket.connect(this->sa, Poco::Timespan(0, timeoutUs));
        }
        catch (const Poco::Exception &ex)
        {
            throw RemoteClientError("Pothos::RemoteClient("+uriStr+")", ex.displayText());
        }
    }
    Poco::Net::StreamSocket clientSocket;
    Poco::Net::SocketStream socketStream;
    const std::string uriStr;
    Poco::Net::SocketAddress sa;
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
    updateNodeIdTable(env, _impl->sa.host()); //update the node id table with this remote host
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
