// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "SocketEndpoint.hpp"
#include <Pothos/Framework.hpp>
#include <cstring> //std::memset
#include <sstream>
#include <string>
#include <cassert>
#include <iostream>

/***********************************************************************
 * |PothosDoc Network Source
 *
 * The network source deserializes data from the socket and produces on its output port.
 * Socket data encompasses stream buffers, inline labels, and async messages.
 *
 * The underlying supports two transport options:
 * TCP - tcp://host:port
 * or UDT - udt://host:port
 *
 * |category /Network
 * |category /Sources
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
 * |factory /blocks/network_source(uri, opt)
 **********************************************************************/
class NetworkSource : public Pothos::Block
{
public:
    static Block *make(const std::string &uri, const std::string &opt)
    {
        return new NetworkSource(uri, opt);
    }

    NetworkSource(const std::string &uri, const std::string &opt):
        _ep(PothosPacketSocketEndpoint(uri, opt)),
        _nextExpectedIndex(0)
    {
        //std::cout << "NetworkSource " << opt << " " << uri << std::endl;
        this->setupOutput(0);
        this->registerCall(this, POTHOS_FCN_TUPLE(NetworkSource, getActualPort));
    }

    std::string getActualPort(void) const
    {
        return _ep.getActualPort();
    }

    void activate(void)
    {
        _ep.openComms();
    }

    void deactivate(void)
    {
        _ep.closeComms();
    }

    void work(void);

private:
    PothosPacketSocketEndpoint _ep;
    unsigned long long _nextExpectedIndex;
    Pothos::DType _lastDtype;
};

void NetworkSource::work(void)
{
    const auto timeout = Poco::Timespan(this->workInfo().maxTimeoutNs/1000);

    auto outputPort = this->output(0);

    //recv the header, use output buffer when possible for zero-copy
    Poco::UInt16 type;
    Poco::UInt64 index;
    auto buffer = outputPort->buffer();
    _ep.recv(type, index, buffer, timeout);

    //handle the output
    if (type == PothosPacketTypeBuffer)
    {
        _nextExpectedIndex = index + buffer.length;
        buffer.dtype = _lastDtype;
        outputPort->popBuffer(buffer.length);
        outputPort->postBuffer(buffer);
    }
    else if (type == PothosPacketTypeMessage)
    {
        std::istringstream iss(std::string(buffer.as<char *>(), buffer.length));
        Pothos::Object msg;
        msg.deserialize(iss);
        outputPort->postMessage(msg);
    }
    else if (type == PothosPacketTypeLabel)
    {
        std::istringstream iss(std::string(buffer.as<char *>(), buffer.length));
        Pothos::Object data;
        data.deserialize(iss);
        auto label = data.extract<Pothos::Label>();
        label.index = index - _nextExpectedIndex;
        outputPort->postLabel(label);
    }
    else if (type == PothosPacketTypeDType)
    {
        std::istringstream iss(std::string(buffer.as<char *>(), buffer.length));
        Pothos::Object data;
        data.deserialize(iss);
        _lastDtype = data.extract<Pothos::DType>();
    }

    return this->yield(); //always yield to service recv() again
}

static Pothos::BlockRegistry registerNetworkSource(
    "/blocks/network_source", &NetworkSource::make);
