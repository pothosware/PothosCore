// Copyright (c) 2013-2016 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PothosUtil.hpp"
#include <Pothos/Init.hpp>
#include <Pothos/Remote.hpp>
#include <Pothos/Util/Network.hpp>
#include <Poco/Net/ServerSocket.h>
#include <Poco/Net/SocketStream.h>
#include <Poco/Net/TCPServer.h>
#include <Poco/Process.h>
#include <Poco/URI.h>
#include <mutex>
#include <cassert>
#include <iostream>

/***********************************************************************
 * TCP connection factory
 *  - create connection handler threads
 *  - monitor connection start and stop
 *  - kill process in require active mode
 **********************************************************************/
class MyTCPServerConnectionFactory : public Poco::Net::TCPServerConnectionFactory
{
public:
    MyTCPServerConnectionFactory(const bool requireActive):
        _numConnections(0),
        _requireActive(requireActive)
    {
        return;
    }

    Poco::Net::TCPServerConnection *createConnection(const Poco::Net::StreamSocket &socket);

    void connectionStart(void)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _numConnections++;
    }

    void connectionStop(void)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        assert(_numConnections != 0);
        _numConnections--;
        if (_numConnections == 0 and _requireActive)
        {
            std::cerr << "Proxy server: No active connections - terminating" << std::endl;
            Poco::Process::requestTermination(Poco::Process::id());
        }
    }

private:
    std::mutex _mutex;
    size_t _numConnections;
    const bool _requireActive;
};

/***********************************************************************
 * TCP connection thread
 *  - create the remote handler for the connection
 *  - service the remote handler until disconnect
 **********************************************************************/
class MyTCPServerConnection : public Poco::Net::TCPServerConnection
{
public:
    MyTCPServerConnection(MyTCPServerConnectionFactory &factory, const Poco::Net::StreamSocket &socket):
        Poco::Net::TCPServerConnection(socket),
        _factory(factory),
        _handler(Pothos::RemoteHandler(socket.peerAddress().host().toString()))
    {
        _factory.connectionStart();
    }

    ~MyTCPServerConnection(void)
    {
        _factory.connectionStop();
    }

    void run(void)
    {
        this->socket().setNoDelay(true);
        Poco::Net::SocketStream socketStream(this->socket());
        _handler.runHandler(socketStream, socketStream);
    }

private:
    MyTCPServerConnectionFactory &_factory;
    Pothos::RemoteHandler _handler;
};

Poco::Net::TCPServerConnection *MyTCPServerConnectionFactory::createConnection(const Poco::Net::StreamSocket &socket)
{
    return new MyTCPServerConnection(*this, socket);
}

/***********************************************************************
 * Spawn TCP proxy server given server URI
 **********************************************************************/
void PothosUtilBase::proxyServer(const std::string &, const std::string &uriStr)
{
    //remove automatic flushing from iostreams
    //only flushes on newlines and intentional flushes
    std::cout << std::nounitbuf;
    std::cerr << std::nounitbuf;
    std::clog << std::nounitbuf;

    //set stdio to be line buffered
    //fully buffered IO backs up and is not acceptable for logging
    setvbuf(stdout, nullptr, _IOLBF, 0);
    setvbuf(stderr, nullptr, _IOLBF, 0);

    Pothos::ScopedInit init;

    //parse the URI
    const std::string defaultUri = "tcp://"+Pothos::Util::getWildcardAddr(Pothos::RemoteServer::getLocatorPort());
    Poco::URI uri(uriStr.empty()?defaultUri:uriStr);
    const std::string &host = uri.getHost();
    const std::string &port = std::to_string(uri.getPort());
    if (uri.getScheme() != "tcp")
    {
        throw Pothos::Exception("PothosUtil::proxyServer("+uriStr+")", "unsupported URI scheme");
    }

    //create server socket
    Poco::Net::SocketAddress sa(host, port);
    Poco::Net::ServerSocket serverSocket(sa);
    Poco::Net::TCPServerConnectionFactory::Ptr factory;
    factory = new MyTCPServerConnectionFactory(this->config().hasOption("requireActive"));
    Poco::Net::TCPServer tcpServer(factory, serverSocket);

    //start the server
    tcpServer.start();
    std::cout << "Host: " << serverSocket.address().host().toString() << std::endl;
    std::cout << "Port: " << serverSocket.address().port() << std::endl;

    //wait here until the term signal is received
    this->waitForTerminationRequest();
}
