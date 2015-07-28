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
 * <h2>Label support</h2>
 *
 * Labels can be used to indicate frame boundaries.
 * When the start and end frame IDs are specified,
 * labels are produced for the first and last output element of each packet payload.
 *
 * The frame start label data holds the number of elements present in the packet.
 * The element count can be used by upstream blocks to determine the length without
 * the use of the end of frame label. A consumer of this label (in most cases)
 * can assume that the number of elements from this label to the end of this packet
 * will be the element count times the label width (which accounts for rate increases).
 *
 * |category /Packet
 * |keywords packet message datagram
 *
 * |param frameStartId[Frame Start ID] The label ID to mark the first element from each payload.
 * An empty string (default) means that start of frame labels are not produced.
 * |default ""
 * |widget StringEntry()
 * |preview valid
 *
 * |param frameEndId[Frame End ID] The label ID to mark the last element from each payload.
 * An empty string (default) means that end of frame labels are not produced.
 * |default ""
 * |widget StringEntry()
 * |preview valid
 *
 * |factory /blocks/packet_to_stream()
 * |setter setFrameStartId(frameStartId)
 * |setter setFrameEndId(frameEndId)
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
        this->registerCall(this, POTHOS_FCN_TUPLE(PacketToStream, setFrameEndId));
        this->registerCall(this, POTHOS_FCN_TUPLE(PacketToStream, getFrameEndId));
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

    void setFrameEndId(std::string id)
    {
        _frameEndId = id;
    }

    std::string getFrameEndId(void) const
    {
        return _frameEndId;
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

        //post start of frame label
        if (not _frameStartId.empty())
        {
            outputPort->postLabel(Pothos::Label(_frameStartId, Pothos::Object(packet.payload.elements()), 0, packet.payload.dtype.size()));
        }

        //post end of frame label
        if (not _frameEndId.empty())
        {
            outputPort->postLabel(Pothos::Label(_frameEndId, Pothos::Object(packet.payload.elements()), packet.payload.length-1, packet.payload.dtype.size()));
        }

        //post the payload
        outputPort->postBuffer(packet.payload);
    }

private:
    std::string _frameStartId;
    std::string _frameEndId;
};

static Pothos::BlockRegistry registerPacketToStream(
    "/blocks/packet_to_stream", &PacketToStream::make);
