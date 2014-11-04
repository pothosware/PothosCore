// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "SocketEndpoint.hpp"
#include <Pothos/Exception.hpp>
#include <Poco/Foundation.h>
#include <Poco/URI.h>
#include <Poco/Format.h>
#include <Poco/Net/StreamSocket.h>
#include <Poco/Net/ServerSocket.h>
#include <Poco/ByteOrder.h>
#include <Poco/SingletonHolder.h>
#include <mutex>
#include <udt.h>
#include <cassert>
#include <iostream>

/***********************************************************************
 * Socket interface abstraction
 **********************************************************************/
struct PothosPacketSocketEndpointInterface
{
    virtual ~PothosPacketSocketEndpointInterface(void){}

    virtual std::string getPort(void) const = 0;

    virtual bool isRecvReady(const std::chrono::high_resolution_clock::duration &timeout) = 0;

    virtual int send(const void *buff, const size_t length) = 0;

    virtual int recv(void *buff, const size_t length) = 0;

};

/***********************************************************************
 * TCP implementation of interface
 **********************************************************************/
struct PothosPacketSocketEndpointInterfaceTcp : PothosPacketSocketEndpointInterface
{
    PothosPacketSocketEndpointInterfaceTcp(const Poco::Net::SocketAddress &addr, const bool server):
        server(server),
        connected(false)
    {
        if (server)
        {
            this->serverSock.bind(addr);
            this->serverSock.listen(1/*only one client expected*/);
        }
        else
        {
            this->clientSock.connect(addr);
            this->clientSock.setNoDelay(true);
            this->connected = true;
        }
    }

    ~PothosPacketSocketEndpointInterfaceTcp(void)
    {
        this->clientSock.close();
        if (server) this->serverSock.close();
    }

    std::string getPort(void) const
    {
        if (server) return std::to_string(serverSock.address().port());
        return std::to_string(clientSock.address().port());
    }

    bool isRecvReady(const std::chrono::high_resolution_clock::duration &timeout)
    {
        if (not connected)
        {
            const auto micros = std::chrono::duration_cast<std::chrono::microseconds>(timeout).count();
            const auto tspan = Poco::Timespan(Poco::Timespan::TimeDiff(micros));
            if (not this->serverSock.poll(tspan, Poco::Net::Socket::SELECT_READ)) return false;
            this->clientSock = this->serverSock.acceptConnection();
            this->clientSock.setNoDelay(true);
            connected = true;
            return false;
        }
        return clientSock.poll(Poco::Timespan(Poco::Timespan::TimeDiff(1e6*0.05)), Poco::Net::Socket::SELECT_READ);
    }

    int send(const void *buff, const size_t length)
    {
        return clientSock.sendBytes(buff, int(length), 0);
    }

    int recv(void *buff, const size_t length)
    {
        return clientSock.receiveBytes(buff, int(length), 0);
    }

    bool server;
    bool connected;
    Poco::Net::ServerSocket serverSock;
    Poco::Net::StreamSocket clientSock;
};

/***********************************************************************
 * UDT implementation of interface
 **********************************************************************/
struct UDTSession
{
    UDTSession(void)
    {
        UDT::startup();
    }
    ~UDTSession(void)
    {
        UDT::cleanup();
    }
};

static std::mutex &getUdtSessionMutex(void)
{
    static Poco::SingletonHolder<std::mutex> sh;
    return *sh.get();
}

static std::shared_ptr<UDTSession> getUDTSession(void)
{
    std::unique_lock<std::mutex> lock(getUdtSessionMutex());
    static std::weak_ptr<UDTSession> UDTWeakSession;
    std::shared_ptr<UDTSession> sess = UDTWeakSession.lock();
    if (not sess)
    {
        sess.reset(new UDTSession());
        UDTWeakSession = sess;
    }
    return sess;
}

struct PothosPacketSocketEndpointInterfaceUdt : PothosPacketSocketEndpointInterface
{
    PothosPacketSocketEndpointInterfaceUdt(const Poco::Net::SocketAddress &addr, const bool server):
        server(server),
        connected(false),
        sess(getUDTSession())
    {
        if (server)
        {
            this->serverSock = makeSocket();
            if (UDT::ERROR == UDT::bind(this->serverSock, addr.addr(), addr.length()))
            {
                throw Pothos::RuntimeException("UDT::bind("+addr.toString()+")", UDT::getlasterror().getErrorMessage());
            }
            UDT::listen(this->serverSock, 1/*only one client expected*/);
        }
        else
        {
            this->clientSock = makeSocket();
            if (UDT::ERROR == UDT::connect(this->clientSock, addr.addr(), addr.length()))
            {
                throw Pothos::RuntimeException("UDT::connect("+addr.toString()+")", UDT::getlasterror().getErrorMessage());
            }
            this->connected = true;
        }
    }

    static UDTSOCKET makeSocket(void)
    {
        UDTSOCKET sock = UDT::socket(AF_INET, SOCK_STREAM, 0);
        //Arbitrary buffer size limit in OSX that must be set on the socket,
        //or UDT will try to set one that is too large and fail bind/connect.
        #if POCO_OS == POCO_OS_MAC_OS_X
        int size = 1024*21;
        UDT::setsockopt(sock, 0, UDP_RCVBUF, &size, int(sizeof(size)));
        #endif
        return sock;
    }

    ~PothosPacketSocketEndpointInterfaceUdt(void)
    {
        UDT::close(clientSock);
        if (server) UDT::close(serverSock);
    }

    std::string getPort(void) const
    {
        sockaddr addr;
        int addrlen = sizeof(addr);
        UDT::getsockname(server?serverSock:clientSock, &addr, &addrlen);
        return std::to_string(Poco::Net::SocketAddress(&addr, addrlen).port());
    }

    bool isRecvReady(const std::chrono::high_resolution_clock::duration &timeout)
    {
        const auto micros = std::chrono::duration_cast<std::chrono::microseconds>(timeout).count();
        struct timeval tv;
        tv.tv_sec = micros / 1000000;
        tv.tv_usec = micros % 1000000;

        if (not connected)
        {
            UDT::UDSET readfds;
            UD_SET(serverSock, &readfds);
            if (UDT::select(1, &readfds, nullptr, nullptr, &tv) != 1) return false;

            sockaddr addr;
            int addrlen = sizeof(addr);
            this->clientSock = UDT::accept(this->serverSock, &addr, &addrlen);
            if (addrlen == 0)
            {
                throw Pothos::RuntimeException("UDT::accept()", UDT::getlasterror().getErrorMessage());
            }
            this->connected = true;
            return false;
        }

        UDT::UDSET readfds;
        UD_SET(clientSock, &readfds);
        return UDT::select(1, &readfds, nullptr, nullptr, &tv) == 1;
    }

    int send(const void *buff, const size_t length)
    {
        int bytes = int(length);
        #ifdef _MSC_VER
        bytes = std::min(bytes, 1024*8);
        #endif
        int r = UDT::send(this->clientSock, (const char *)buff, bytes, 0);
        if (r == UDT::ERROR) throw Pothos::RuntimeException("UDT::send()", UDT::getlasterror().getErrorMessage());
        return r;
    }

    int recv(void *buff, const size_t length)
    {
        int r = UDT::recv(this->clientSock, (char *)buff, int(length), 0);
        if (r == UDT::ERROR) throw Pothos::RuntimeException("UDT::recv()", UDT::getlasterror().getErrorMessage());
        return r;
    }

    bool server;
    bool connected;
    UDTSOCKET serverSock;
    UDTSOCKET clientSock;
    std::shared_ptr<UDTSession> sess;
};

/***********************************************************************
 * Protocol header format
 **********************************************************************/
#define POTHOS_PACKET_WORD32(str) \
    (uint32_t(str[0]) << 24) | \
    (uint32_t(str[1]) << 16) | \
    (uint32_t(str[2]) << 8) | \
    (uint32_t(str[3]) << 0)

static const uint32_t PothosPacketHeaderWord = POTHOS_PACKET_WORD32("PTHS");

#define PothosPacketFlagFin (1 << 0)
#define PothosPacketFlagSyn (1 << 1)
#define PothosPacketFlagRst (1 << 2)
#define PothosPacketFlagPsh (1 << 3)
#define PothosPacketFlagAck (1 << 4)
#define PothosPacketFlagFlo (1 << 5)

struct PothosPacketHeader
{
    uint32_t headerWord;
    uint16_t flags;
    uint16_t type;
    uint16_t payloadBytes;
    uint16_t packetCount;
    uint32_t indexWord[2];
};

/***********************************************************************
 * States for connection establishment and termination
 **********************************************************************/
enum EndpointState
{
    EP_STATE_LISTEN,
    EP_STATE_SYN_SENT,
    EP_STATE_SYN_RECEIVED,
    EP_STATE_ESTABLISHED,
    EP_STATE_FIN_WAIT_1,
    EP_STATE_FIN_WAIT_2,
    EP_STATE_CLOSE_WAIT,
    EP_STATE_CLOSING,
    EP_STATE_LAST_ACK,
    EP_STATE_TIME_WAIT,
    EP_STATE_CLOSED,
};

/***********************************************************************
 * Private implementation guts
 **********************************************************************/
struct PothosPacketSocketEndpoint::Impl
{
    Impl(void):
        state(EP_STATE_CLOSED),
        lastSentPacketCount(0),
        nextRecvPacketCount(0),
        bytesLeftInStream(0),
        iface(nullptr)
    {
        return;
    }

    //state
    EndpointState state;
    uint16_t lastSentPacketCount;
    uint16_t nextRecvPacketCount;
    size_t bytesLeftInStream;
    uint16_t lastType;
    uint64_t lastIndex;
    Poco::Net::SocketAddress actualAddr;
    uint64_t totalBytesRecv;
    uint64_t totalBytesSent;
    uint64_t lastFlowMsgRecv;
    uint64_t lastFlowMsgSent;

    PothosPacketSocketEndpointInterface *iface;

    void unpackHeader(const PothosPacketHeader &header, const size_t recvBytes, uint16_t &flags, uint16_t &type, uint64_t &index, size_t &payloadBytes);
    void handleState(const uint16_t &flags);
    void send(const uint16_t flags)
    {
        return this->send(flags, 0, 0, nullptr, 0);
    }
    void send(const uint16_t flags, const uint16_t type, const uint64_t &index, const void *buff, const size_t numBytes);
    void recv(uint16_t &flags, uint16_t &type, uint64_t &index, Pothos::BufferChunk &buffer, const std::chrono::high_resolution_clock::duration &timeout);

    uint64_t flowControlWindowBytes(void) const
    {
        return 256*1024;
    }

    std::mutex sendMutex;
};

/***********************************************************************
 * endpoint constructor
 **********************************************************************/
PothosPacketSocketEndpoint::PothosPacketSocketEndpoint(const std::string &uri, const std::string &opt):
    _impl(new Impl())
{
    if (opt == "DISCONNECT") return;
    if (opt == "BIND") _impl->state = EP_STATE_LISTEN;
    if (opt == "CONNECT") _impl->state = EP_STATE_CLOSED;
    try
    {
        Poco::URI uriObj(uri);
        const Poco::Net::SocketAddress addr(uriObj.getHost(), uriObj.getPort());
        if (uriObj.getScheme() == "tcp" and opt == "BIND")
        {
            _impl->iface = new PothosPacketSocketEndpointInterfaceTcp(addr, true);
        }
        else if (uriObj.getScheme() == "tcp" and opt == "CONNECT")
        {
            _impl->iface = new PothosPacketSocketEndpointInterfaceTcp(addr, false);
        }
        else if (uriObj.getScheme() == "udt" and opt == "BIND")
        {
            _impl->iface = new PothosPacketSocketEndpointInterfaceUdt(addr, true);
        }
        else if (uriObj.getScheme() == "udt" and opt == "CONNECT")
        {
            _impl->iface = new PothosPacketSocketEndpointInterfaceUdt(addr, false);
        }
        else
        {
            throw Pothos::InvalidArgumentException("PothosPacketSocketEndpoint("+uri+" -> "+opt+")",
                "unknown URI scheme + opt combo, expects tcp/udp, CONNECT/BIND");
        }
    }
    catch (const Poco::Exception &ex)
    {
        throw Pothos::RuntimeException("PothosPacketSocketEndpoint("+uri+" -> "+opt+")", ex.displayText());
    }
}

PothosPacketSocketEndpoint::~PothosPacketSocketEndpoint(void)
{
    try
    {
        this->closeComms();
    }
    catch (...)
    {
        //failure OK, other endpoint may be destructed
    }
    delete _impl->iface;
    delete _impl;
}

std::string PothosPacketSocketEndpoint::getActualPort(void) const
{
    return _impl->iface->getPort();
}

bool PothosPacketSocketEndpoint::isReady(void)
{
    return (_impl->state == EP_STATE_ESTABLISHED) and
        (_impl->lastFlowMsgRecv + _impl->flowControlWindowBytes() > _impl->totalBytesSent);
}

/***********************************************************************
 * initiate open transactions
 **********************************************************************/
void PothosPacketSocketEndpoint::openComms(void)
{
    Pothos::BufferChunk buffer(1024);
    uint16_t type;
    uint64_t index;

    //start with a new random sequence number
    _impl->lastSentPacketCount = uint16_t(std::rand());
    _impl->totalBytesRecv = 0;
    _impl->totalBytesSent = 0;
    _impl->lastFlowMsgRecv = 0;
    _impl->lastFlowMsgSent = 0;

    //initiate connect operation
    if (_impl->state == EP_STATE_CLOSED)
    {
        _impl->send(PothosPacketFlagSyn);
        _impl->state = EP_STATE_SYN_SENT;
    }

    //loop until timeout (we will exit before timeout under normal conditions)
    const auto exitTime = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(500); //half a second
    while (std::chrono::high_resolution_clock::now() < exitTime)
    {
        if (_impl->state == EP_STATE_ESTABLISHED) break;
        if (_impl->state == EP_STATE_CLOSED) break;
        this->recv(type, index, buffer);
    }

    //check the state on loop exit
    if (_impl->state != EP_STATE_ESTABLISHED)
    {
        _impl->state = EP_STATE_CLOSED;
        throw Pothos::RuntimeException("PothosPacketSocketEndpoint::openComms()", "handshake failed");
    }
}

/***********************************************************************
 * initiate close transactions
 **********************************************************************/
void PothosPacketSocketEndpoint::closeComms(void)
{
    if (_impl->state == EP_STATE_CLOSED) return;

    Pothos::BufferChunk buffer(1024);
    uint16_t type;
    uint64_t index;

    //initiate a close operation
    switch (_impl->state)
    {
    case EP_STATE_ESTABLISHED:
        _impl->send(PothosPacketFlagFin);
        _impl->state = EP_STATE_FIN_WAIT_1;
        break;
    case EP_STATE_CLOSE_WAIT:
        _impl->send(PothosPacketFlagFin);
        _impl->state = EP_STATE_LAST_ACK;
        break;
    default:
        _impl->state = EP_STATE_CLOSED;
    }

    //loop until timeout (we will exit before timeout under normal conditions)
    const auto exitTime = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(500); //half a second
    while (std::chrono::high_resolution_clock::now() < exitTime)
    {
        if (_impl->state == EP_STATE_TIME_WAIT) _impl->state = EP_STATE_CLOSED;
        if (_impl->state == EP_STATE_CLOSED) break;
        this->recv(type, index, buffer);
    }

    //check the state on loop exit
    if (_impl->state != EP_STATE_CLOSED)
    {
        throw Pothos::RuntimeException("PothosPacketSocketEndpoint::closeComms()", "handshake failed");
    }
}

/***********************************************************************
 * handle connection state
 **********************************************************************/
void PothosPacketSocketEndpoint::Impl::handleState(const uint16_t &flags)
{
    switch (this->state)
    {
    case EP_STATE_LISTEN:
        if ((flags & PothosPacketFlagSyn) != 0)
        {
            this->send(PothosPacketFlagSyn | PothosPacketFlagAck);
            this->state = EP_STATE_SYN_RECEIVED;
        }
        break;

    case EP_STATE_SYN_SENT:
        if ((flags & (PothosPacketFlagSyn | PothosPacketFlagAck)) != 0)
        {
            this->send(PothosPacketFlagAck);
            this->state = EP_STATE_ESTABLISHED;
        }
        else if ((flags & PothosPacketFlagSyn) != 0)
        {
            this->send(PothosPacketFlagSyn | PothosPacketFlagAck);
            this->state = EP_STATE_SYN_RECEIVED;
        }
        break;

    case EP_STATE_SYN_RECEIVED:
        if ((flags & PothosPacketFlagAck) != 0)
        {
            this->state = EP_STATE_ESTABLISHED;
        }
        break;

    case EP_STATE_ESTABLISHED:
        if ((flags & PothosPacketFlagFin) != 0)
        {
            this->send(PothosPacketFlagAck);
            this->state = EP_STATE_CLOSE_WAIT;
        }
        break;

    case EP_STATE_FIN_WAIT_1:
        if ((flags & PothosPacketFlagAck) != 0)
        {
            this->state = EP_STATE_FIN_WAIT_2;
        }
        else if ((flags & PothosPacketFlagFin) != 0)
        {
            this->send(PothosPacketFlagAck);
            this->state = EP_STATE_CLOSING;
        }
        break;

    case EP_STATE_FIN_WAIT_2:
        if ((flags & PothosPacketFlagFin) != 0)
        {
            this->send(PothosPacketFlagAck);
            this->state = EP_STATE_TIME_WAIT;
        }
        break;

    case EP_STATE_CLOSE_WAIT:
        //handled by close comms
        break;

    case EP_STATE_CLOSING:
        if ((flags & PothosPacketFlagAck) != 0)
        {
            this->state = EP_STATE_TIME_WAIT;
        }
        break;

    case EP_STATE_LAST_ACK:
        if ((flags & PothosPacketFlagAck) != 0)
        {
            this->state = EP_STATE_CLOSED;
        }
        break;

    case EP_STATE_TIME_WAIT:
        //handled by close comms
        break;

    case EP_STATE_CLOSED:
        break;

    }
}

/***********************************************************************
 * handler/parser for received buffers
 **********************************************************************/
void PothosPacketSocketEndpoint::Impl::unpackHeader(const PothosPacketHeader &header, const size_t recvBytes, uint16_t &flags, uint16_t &type, uint64_t &index, size_t &payloadBytes)
{
    if (recvBytes < sizeof(header))
    {
        throw Pothos::Exception("PothosPacketSocketEndpoint::unpackHeader()", "incomplete header");
    }

    if (Poco::ByteOrder::fromNetwork(header.headerWord) != PothosPacketHeaderWord)
    {
        throw Pothos::Exception("PothosPacketSocketEndpoint::unpackHeader()", "headerWord fail");
    }

    //extract header fields
    flags = Poco::ByteOrder::fromNetwork(header.flags);
    const uint16_t recvPacketCount = Poco::ByteOrder::fromNetwork(header.packetCount);
    payloadBytes = Poco::ByteOrder::fromNetwork(header.payloadBytes);
    type = Poco::ByteOrder::fromNetwork(header.type);
    index = uint64_t(Poco::ByteOrder::fromNetwork(header.indexWord[1]));
    index |= (uint64_t(Poco::ByteOrder::fromNetwork(header.indexWord[0])) << 32);

    //when the sender is telling us to use a new sequence number
    if ((flags & PothosPacketFlagSyn) != 0) this->nextRecvPacketCount = recvPacketCount;

    //always must be correct
    if (recvPacketCount != this->nextRecvPacketCount)
    {
        throw Pothos::Exception("PothosPacketSocketEndpoint::unpackHeader()", "packetCount fail");
    }

    //increment for next packet
    this->nextRecvPacketCount = recvPacketCount + 1;

    //save header fields for partial recvs
    lastType = type;
    lastIndex = index;

    //run the handler for the state machine
    this->handleState(flags);
}

/***********************************************************************
 * perform a recv operation on the connected socket
 **********************************************************************/
void PothosPacketSocketEndpoint::recv(uint16_t &type, uint64_t &index, Pothos::BufferChunk &buffer, const std::chrono::high_resolution_clock::duration &timeout)
{
    uint16_t flags = 0;
    return _impl->recv(flags, type, index, buffer, timeout);
}

void PothosPacketSocketEndpoint::Impl::recv(uint16_t &flags, uint16_t &type, uint64_t &index, Pothos::BufferChunk &buffer, const std::chrono::high_resolution_clock::duration &timeout)
{
    flags = 0;
    type = 0;
    index = 0;

    if (not this->iface->isRecvReady(timeout)) return;

    int ret;
    PothosPacketHeader header;

    //no bytes left in stream, receive a new header
    if (this->bytesLeftInStream == 0)
    {
        //receive the header
        ret = this->iface->recv(&header, sizeof(header));
        if (ret <= 0)
        {
            throw Pothos::Exception("PothosPacketSocketEndpoint::recv(header)", std::to_string(ret));
        }
        this->totalBytesRecv += ret;

        //extract header fields
        this->unpackHeader(header, size_t(ret), flags, type, index, this->bytesLeftInStream);

        //create a new buffer of the required length if need be
        //partial receives are always ok with packet buffer type
        if (type != PothosPacketTypeBuffer and buffer.length < this->bytesLeftInStream)
        {
            buffer = Pothos::BufferChunk(this->bytesLeftInStream);
        }
    }

    //otherwise, restore from the last recv()
    else
    {
        flags = 0;
        type = lastType;
        index = lastIndex;
    }

    //receive the payload into the available buffer
    size_t bytesRecvd = 0;
    buffer.length = std::min(buffer.length, this->bytesLeftInStream);
    while (buffer.length > bytesRecvd)
    {
        ret = this->iface->recv((buffer.as<char *>() + bytesRecvd), buffer.length-bytesRecvd);
        if (ret <= 0)
        {
            throw Pothos::Exception("PothosPacketSocketEndpoint::recv(payload)", std::to_string(ret));
        }
        this->totalBytesRecv += ret;
        bytesRecvd += size_t(ret);
    }

    this->bytesLeftInStream -= buffer.length;

    //deal with flow control (incoming)
    if ((flags & PothosPacketFlagFlo) != 0)
    {
        this->lastFlowMsgRecv = index;
    }

    //deal with flow control (outgoing)
    if (this->totalBytesRecv > this->lastFlowMsgSent + this->flowControlWindowBytes()/8)
    {
        this->send(PothosPacketFlagFlo, 0, this->totalBytesRecv, nullptr, 0);
        this->lastFlowMsgSent = this->totalBytesRecv;
    }
}

/***********************************************************************
 * perform a send operation on the connected socket
 **********************************************************************/
void PothosPacketSocketEndpoint::send(const uint16_t type, const uint64_t &index, const void *buff, const size_t numBytes)
{
    _impl->send(PothosPacketFlagPsh, type, index, buff, numBytes);
}

void PothosPacketSocketEndpoint::Impl::send(const uint16_t flags, const uint16_t type, const uint64_t &index, const void *buff, const size_t numBytes)
{
    std::unique_lock<std::mutex> lock(this->sendMutex);

    int ret;
    PothosPacketHeader header;
    header.headerWord = Poco::ByteOrder::toNetwork(PothosPacketHeaderWord);
    header.flags = Poco::ByteOrder::toNetwork(flags);
    header.payloadBytes = Poco::ByteOrder::toNetwork(uint16_t(numBytes));
    header.packetCount = Poco::ByteOrder::toNetwork(uint16_t(this->lastSentPacketCount++));
    header.type = Poco::ByteOrder::toNetwork(type);
    header.indexWord[0] = Poco::ByteOrder::toNetwork(uint32_t(index >> 32));
    header.indexWord[1] = Poco::ByteOrder::toNetwork(uint32_t(index >> 0));

    //send the header
    ret = this->iface->send(&header, sizeof(header));
    if (ret != int(sizeof(header)))
    {
        throw Pothos::Exception("PothosPacketSocketEndpoint::send(header)", std::to_string(ret));
    }
    this->totalBytesSent += ret;

    //send all of the available buffer
    size_t bytesLeft = numBytes;
    while (bytesLeft != 0)
    {
        ret = this->iface->send((const void *)(size_t(buff)+numBytes-bytesLeft), bytesLeft);
        if (ret <= 0)
        {
            throw Pothos::Exception("PothosPacketSocketEndpoint::send(payload)", std::to_string(ret));
        }
        this->totalBytesSent += ret;
        bytesLeft -= size_t(ret);
    }
}
