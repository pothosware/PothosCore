// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "network/SocketEndpoint.hpp"
#include <Pothos/Framework.hpp>
#include <thread>
#include <sstream>
#include <string>
#include <cassert>
#include <cassert>

/***********************************************************************
 * |PothosDoc Network Sink
 *
 * The network sink accepts data on its input port and serializes it over a socket.
 * All input port data is serialized, which includes stream buffers, inline labels, and async messages.
 *
 * The underlying supports two transport options:
 * TCP - tcp://host:port
 * or UDT - udt://host:port
 *
 * |category /Network
 * |keywords source network
 *
 * |param uri[URI] The bind or connection uri string.
 * |default "udt://192.168.10.2:1234"
 *
 * |param opt[Option] Control if the socket is a server (BIND) or client (CONNECT).
 * The "DISCONNECT" option is used to make a disconnected endpoint for object inspection.
 * |option [Disconnect] "DISCONNECT"
 * |option [Connect] "CONNECT"
 * |option [Bind] "BIND"
 * |default "DISCONNECT"
 *
 * |param dtype[Data Type] The datatype consumed by the network sink.
 * |default "int"
 *
 * |factory /blocks/network/network_sink(uri, opt, dtype)
 **********************************************************************/
class NetworkSink : public Pothos::Block
{
public:
    static Block *make(const std::string &uri, const std::string &opt, const Pothos::DType &dtype)
    {
        return new NetworkSink(uri, opt, dtype);
    }

    NetworkSink(const std::string &uri, const std::string &opt, const Pothos::DType &dtype):
        _ep(PothosPacketSocketEndpoint(uri, opt)),
        running(false)
    {
        //std::cout << "NetworkSink " << opt << " " << uri << std::endl;
        this->setupInput(0, dtype);
        this->registerCall(POTHOS_FCN_TUPLE(NetworkSink, getActualPort));
    }

    std::string getActualPort(void) const
    {
        return _ep.getActualPort();
    }

    void activate(void)
    {
        _ep.openComms();

        //start the endpoint handler thread
        running = true;
        assert(not handlerThread.joinable());
        handlerThread = std::thread(&NetworkSink::handleState, this);
    }

    void deactivate(void)
    {
        //stop the endpoint handler thread
        running = false;
        assert(handlerThread.joinable());
        handlerThread.join();

        _ep.closeComms();
    }

    //NetworkSink is a send-only block and needs a polling thread.
    void handleState(void)
    {
        while (running)
        {
            Poco::UInt16 type;
            Poco::UInt64 index;
            Pothos::BufferChunk buffer;
            _ep.recv(type, index, buffer);
        }
    }

    void work(void);

private:
    PothosPacketSocketEndpoint _ep;
    std::thread handlerThread;
    bool running;
};

void NetworkSink::work(void)
{
    if (not _ep.isReady()) return;

    const auto timeout = Poco::Timespan(this->workInfo().maxTimeoutNs/1000);

    auto inputPort = this->inputs()[0];

    //serialize messages
    while (inputPort->hasMessage())
    {
        std::ostringstream oss;
        inputPort->popMessage().serialize(oss);
        _ep.send(PothosPacketTypeMessage, inputPort->totalMessages(), oss.str().data(), oss.str().length());
    }

    //serialize labels (all labels are sent before buffers to ensure ordering at the destination)
    while (inputPort->labels().begin() != inputPort->labels().end())
    {
        const auto &label = *inputPort->labels().begin();
        std::ostringstream oss;
        label.data.serialize(oss);
        _ep.send(PothosPacketTypeLabel, label.index, oss.str().data(), oss.str().length());
        inputPort->removeLabel(label);
    }

    //send a buffer
    const auto &buffer = inputPort->buffer();
    if (buffer.length != 0)
    {
        _ep.send(PothosPacketTypeBuffer, inputPort->totalElements(), buffer.as<const void *>(), buffer.length);
        inputPort->consume(inputPort->elements());
    }
}

static Pothos::BlockRegistry registerNetworkSink(
    "/blocks/network/network_sink", &NetworkSink::make);
