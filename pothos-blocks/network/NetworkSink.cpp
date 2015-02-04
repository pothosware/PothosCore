// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "SocketEndpoint.hpp"
#include <Pothos/Framework.hpp>
#include <thread>
#include <sstream>
#include <string>
#include <chrono>
#include <cassert>
#include <iostream>

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
 * |category /Sinks
 * |keywords sink network
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
 * |factory /blocks/network_sink(uri, opt)
 **********************************************************************/
class NetworkSink : public Pothos::Block
{
public:
    static Block *make(const std::string &uri, const std::string &opt)
    {
        return new NetworkSink(uri, opt);
    }

    NetworkSink(const std::string &uri, const std::string &opt):
        _ep(PothosPacketSocketEndpoint(uri, opt)),
        running(false)
    {
        //std::cout << "NetworkSink " << opt << " " << uri << std::endl;
        this->setupInput(0);
        this->registerCall(this, POTHOS_FCN_TUPLE(NetworkSink, getActualPort));
    }

    ~NetworkSink(void)
    {
        //the thread cannot be left running
        if (handlerThread.joinable())
        {
            running = false;
            handlerThread.join();
        }
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
        assert(handlerThread.joinable());
        running = false;
        handlerThread.join();

        _ep.closeComms();
    }

    //NetworkSink is a send-only block and needs a polling thread.
    void handleState(void)
    {
        while (running)
        {
            uint16_t type;
            uint64_t index;
            Pothos::BufferChunk buffer;
            try {_ep.recv(type, index, buffer);}
            catch (...){}
        }
    }

    void work(void);

    void updateDType(const Pothos::DType &dtype)
    {
        if (_lastDtype == dtype) return;
        std::ostringstream oss;
        Pothos::Object(dtype).serialize(oss);
        _ep.send(PothosPacketTypeDType, 0, oss.str().data(), oss.str().length(), true);
        _lastDtype = dtype;
    }

private:
    PothosPacketSocketEndpoint _ep;
    std::thread handlerThread;
    bool running;
    Pothos::DType _lastDtype;
};

void NetworkSink::work(void)
{
    if (not _ep.isReady())
    {
        std::this_thread::sleep_for(std::chrono::nanoseconds(this->workInfo().maxTimeoutNs));
        return this->yield();
    }

    auto inputPort = this->input(0);

    //serialize messages
    while (inputPort->hasMessage())
    {
        const auto msg = inputPort->popMessage();

        //special efficient packing for buffers in the packet
        if (msg.type() == typeid(Pothos::Packet))
        {
            //extract packet and clear its payload (just send header)
            auto packet = msg.extract<Pothos::Packet>();
            const auto buffer = packet.payload;
            packet.payload = Pothos::BufferChunk();

            //send the packet without buffer
            std::ostringstream oss; Pothos::Object(packet).serialize(oss);
            _ep.send(PothosPacketTypeHeader, inputPort->totalMessages(), oss.str().data(), oss.str().length(), true);

            //send the dtype when changed
            this->updateDType(buffer.dtype);

            //send the packet buffer
            _ep.send(PothosPacketTypePayload, 0, buffer.as<const void *>(), buffer.length);
        }

        //arbitrary serialization
        else
        {
            std::ostringstream oss;
            msg.serialize(oss);
            _ep.send(PothosPacketTypeMessage, inputPort->totalMessages(), oss.str().data(), oss.str().length());
        }
    }

    //serialize labels (all labels are sent before buffers to ensure ordering at the destination)
    while (inputPort->labels().begin() != inputPort->labels().end())
    {
        const auto &label = *inputPort->labels().begin();
        std::ostringstream oss;
        Pothos::Object(label).serialize(oss);
        auto index = label.index + inputPort->totalElements();
        _ep.send(PothosPacketTypeLabel, index, oss.str().data(), oss.str().length());
        inputPort->removeLabel(label);
    }

    //available buffer?
    const auto &buffer = inputPort->buffer();
    if (buffer.length == 0) return;

    //send the dtype when changed
    this->updateDType(buffer.dtype);

    //send a buffer
    {
        _ep.send(PothosPacketTypeBuffer, inputPort->totalElements(), buffer.as<const void *>(), buffer.length);
        inputPort->consume(inputPort->elements());
    }
}

static Pothos::BlockRegistry registerNetworkSink(
    "/blocks/network_sink", &NetworkSink::make);
