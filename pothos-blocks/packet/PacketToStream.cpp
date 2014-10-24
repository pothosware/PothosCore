// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>

/***********************************************************************
 * |PothosDoc Packet To Stream
 *
 * The packet to stream block is a simple translator between
 * message-based packetized input and a byte stream output.
 * The block accepts Pothos::Packet message objects on input port 0,
 * and converts the messages into a byte stream with labels.
 * The byte stream and labels are produced on output port 0.
 *
 * If the input port 0 has a non-packet message,
 * it will be forwarded directly to output port 0.
 *
 * This is zero-copy block implementation.
 * Input buffer references held by the packet object
 * will be forwarded directly to the output byte stream.
 *
 * |category /Packet
 * |keywords packet message datagram
 *
 * |factory /blocks/packet_to_stream()
 **********************************************************************/
class PacketToStream : public Pothos::Block
{
public:
    PacketToStream(void)
    {
        this->setupInput(0);
        this->setupOutput(0, "", this->uid()/*unique domain*/);
    }

    static Block *make(void)
    {
        return new PacketToStream();
    }

    void work(void)
    {
        auto inputPort = this->input(0);
        auto outputPort = this->output(0);

        //extract message
        if (not inputPort->hasMessage()) return;
        auto msg = inputPort->popMessage();

        //forward non-packet messages
        if (msg.type() != typeid(Pothos::Packet))
        {
            outputPort->postMessage(msg);
            return;
        }
        const auto &packet = msg.extract<Pothos::Packet>();

        //post output labels
        for (auto label : packet.labels)
        {
            label.index *= packet.payload.dtype.size(); //elements to bytes
            outputPort->postLabel(label);
        }

        //post the payload
        outputPort->postBuffer(packet.payload);
    }
};

static Pothos::BlockRegistry registerPacketToStream(
    "/blocks/packet_to_stream", &PacketToStream::make);
