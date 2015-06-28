// Copyright (c) 2014-2015 Josh Blum
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
 * Use of labels to indicate start of frame: When the frame start ID is specified,
 * a new label is produced for the first output element from each packet payload.
 *
 * |category /Packet
 * |keywords packet message datagram
 *
 * |param frameStartId[Frame Start ID] The label ID to mark the first element from each payload.
 * An empty string (default) means that frame labels are not produced.
 * |default ""
 * |widget StringEntry()
 * |preview valid
 *
 * |factory /blocks/packet_to_stream()
 * |setter setFrameStartId(frameStartId)
 **********************************************************************/
class PacketToStream : public Pothos::Block
{
public:
    PacketToStream(void)
    {
        this->setupInput(0);
        this->setupOutput(0, "", this->uid()/*unique domain*/);
        this->registerCall(this, POTHOS_FCN_TUPLE(PacketToStream, setFrameStartId));
        this->registerCall(this, POTHOS_FCN_TUPLE(PacketToStream, getFrameStartId));
    }

    static Block *make(void)
    {
        return new PacketToStream();
    }

    void setFrameStartId(std::string id)
    {
        _frameStartId = id;
    }

    std::string getFrameStartId(void) const
    {
        return _frameStartId;
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
            outputPort->postLabel(label.toAdjusted(
                packet.payload.dtype.size(), 1)); //elements to bytes
        }

        //post frame label
        if (not _frameStartId.empty())
        {
            outputPort->postLabel(Pothos::Label(_frameStartId, Pothos::Object(), 0, packet.payload.dtype.size()));
        }

        //post the payload
        outputPort->postBuffer(packet.payload);
    }

private:
    std::string _frameStartId;
};

static Pothos::BlockRegistry registerPacketToStream(
    "/blocks/packet_to_stream", &PacketToStream::make);
