// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "network/SocketEndpoint.hpp"
#include <Pothos/Framework.hpp>
#include <cstring> //std::memset
#include <sstream>
#include <string>
#include <cassert>
#include <iostream>

/***********************************************************************
 * NetworkSource Implementation
 **********************************************************************/
class NetworkSource : public Pothos::Block
{
public:
    static Block *make(const std::string &uri, const std::string &opt, const Pothos::DType &dtype)
    {
        return new NetworkSource(uri, opt, dtype);
    }

    NetworkSource(const std::string &uri, const std::string &opt, const Pothos::DType &dtype):
        _ep(PothosPacketSocketEndpoint(uri, opt))
    {
        //std::cout << "NetworkSource " << opt << " " << uri << std::endl;
        this->setupOutput(0, dtype);
        this->registerCall(POTHOS_FCN_TUPLE(NetworkSource, getActualPort));
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
};

void NetworkSource::work(void)
{
    const auto timeout = Poco::Timespan(this->workInfo().maxTimeoutNs/1000);

    auto outputPort = this->outputs()[0];

    //recv the header, use output buffer when possible for zero-copy
    Poco::UInt16 type;
    Poco::UInt64 index;
    auto buffer = outputPort->buffer();
    _ep.recv(type, index, buffer, timeout);

    //handle the output
    if (type == PothosPacketTypeBuffer)
    {
        //there was a drop, post recovery padding
        if (index > outputPort->totalElements())
        {
            assert(outputPort->totalElements() < index);
            Pothos::BufferChunk recovery((index - outputPort->totalElements())*outputPort->dtype().size());
            std::memset(recovery.as<void *>(), 0, recovery.length);
            outputPort->postBuffer(recovery);
        }
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
        Pothos::Label label;
        label.index = index;
        label.data.deserialize(iss);
        outputPort->postLabel(label);
    }
}

static Pothos::BlockRegistry registerNetworkSource(
    "/blocks/network/network_source", &NetworkSource::make);
