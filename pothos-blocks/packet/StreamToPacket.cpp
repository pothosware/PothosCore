// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <algorithm> //min/max

/***********************************************************************
 * |PothosDoc Stream To Packet
 *
 * The stream to packet block is a simple translator between
 * a byte stream input and a message-based packetized output.
 * The block accepts a byte stream with labels on input port 0,
 * and converts the stream into Pothos::Packet message objects.
 * The packet message objects are then produced on output port 0.
 *
 * If the input port 0 has an incoming message,
 * it will be forwarded directly to output port 0.
 *
 * This is zero-copy block implementation.
 * The output packet object holds a reference to the input stream buffer,
 * without incurring a copy of the buffer.
 *
 * <h2>MTU usage</h2>
 * The MTU can be used to control the maximum size packet payload.
 * When specified, the packet payloads cannot exceed the MTU size.
 * However, packet payloads may be smaller than the specified MTU
 * if the available buffer was smaller.
 * When the MTU is unspecified, the entire available buffer is used.
 *
 * <h2>Label support</h2>
 *
 * Labels can be used to indicate frame boundaries,
 * and to discard stream data located outside of the boundaries.
 *
 * <b>Default operation:</b>
 * In the default operation mode, no frame IDs are specified.
 * The stream to packet block accepts and forwards the input buffer.
 *
 * <b>Start-frame operation:</b>
 * In the start-frame operation mode, no end of frame ID is specified.
 * The stream to packet block drops all input data
 * until a start of frame label is encountered.
 * Unlike the other modes which treat MTU as a maximum length,
 * this mode produces an exact MTU length payload for every start of frame.
 *
 * <b>Full-frame operation:</b>
 * In full-frame operation mode, both frame IDs are specified.
 * The stream to packet block drops all input data
 * until a start of frame label is encountered.
 * After that, multiple packet payloads are produced
 * until an end of frame label is encountered.
 *
 * |category /Packet
 * |keywords packet message datagram
 *
 * |param mtu[MTU] The maximum size of the payload in an output packet.
 * An MTU of 0 bytes means unconstrained payload size;
 * packet payloads will accept the entire input buffer.
 * |default 0
 * |units bytes
 * |preview valid
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
 * |factory /blocks/stream_to_packet()
 * |setter setMTU(mtu)
 * |setter setFrameStartId(frameStartId)
 * |setter setFrameEndId(frameEndId)
 **********************************************************************/
class StreamToPacket : public Pothos::Block
{
public:
    StreamToPacket(void):
        _mtu(0),
        _inFrame(false),
        _startFrameMode(false),
        _fullFrameMode(false)
    {
        this->setupInput(0);
        this->setupOutput(0);
        this->registerCall(this, POTHOS_FCN_TUPLE(StreamToPacket, setMTU));
        this->registerCall(this, POTHOS_FCN_TUPLE(StreamToPacket, getMTU));
        this->registerCall(this, POTHOS_FCN_TUPLE(StreamToPacket, setFrameStartId));
        this->registerCall(this, POTHOS_FCN_TUPLE(StreamToPacket, getFrameStartId));
        this->registerCall(this, POTHOS_FCN_TUPLE(StreamToPacket, setFrameEndId));
        this->registerCall(this, POTHOS_FCN_TUPLE(StreamToPacket, getFrameEndId));
    }

    static Block *make(void)
    {
        return new StreamToPacket();
    }

    void setMTU(const size_t mtu)
    {
        _mtu = mtu;
    }

    size_t getMTU(void) const
    {
        return _mtu;
    }

    void setFrameStartId(std::string id)
    {
        _frameStartId = id;
        this->updateModes();
    }

    std::string getFrameStartId(void) const
    {
        return _frameStartId;
    }

    void setFrameEndId(std::string id)
    {
        _frameEndId = id;
        this->updateModes();
    }

    std::string getFrameEndId(void) const
    {
        return _frameEndId;
    }

    void activate(void)
    {
        _inFrame = false; //reset state
    }

    void work(void)
    {
        auto inputPort = this->input(0);
        auto outputPort = this->output(0);

        //forward messages
        while (inputPort->hasMessage())
        {
            auto msg = inputPort->popMessage();
            outputPort->postMessage(msg);
        }

        //is there any input buffer available?
        if (inputPort->elements() == 0) return;

        //start frame mode has its own work implementation
        if (_startFrameMode) return this->startFrameModeWork();

        //drop until start of frame label
        if (_fullFrameMode and not _inFrame)
        {
            for (const auto &label : inputPort->labels())
            {
                //end of input buffer labels, consume all, done work
                if (label.index >= inputPort->elements())
                {
                    inputPort->consume(inputPort->elements());
                    return;
                }

                //ignore labels that are not start of frame
                if (label.id != _frameStartId) continue;

                //in position 0, set in frame, done loop
                if (label.index == 0)
                {
                    _inFrame = true;
                    break;
                }

                //otherwise consume up to but not including
                //done work, will be in-frame for next work
                else
                {
                    inputPort->consume(label.index);
                    _inFrame = true;
                    return;
                }
            }
        }

        //grab the input buffer
        Pothos::Packet packet;
        packet.payload = inputPort->buffer();
        if (_mtu != 0)
        {
            const auto elemSize = packet.payload.dtype.size();
            const auto mtuElems = (_mtu/elemSize)*elemSize;
            packet.payload.length = std::min(mtuElems, packet.payload.length);
        }

        //grab the input labels
        for (const auto &label : inputPort->labels())
        {
            const auto pktLabel = label.toAdjusted(
                1, packet.payload.dtype.size()); //bytes to elements
            if (pktLabel.index >= packet.payload.elements()) break;
            packet.labels.push_back(pktLabel);

            //end of frame found, truncate payload and leave loop
            if (_fullFrameMode and label.id == _frameEndId)
            {
                packet.payload.length = label.index+label.width;
                _inFrame = false;
                break;
            }
        }

        //consume the input and produce the packet
        inputPort->consume(packet.payload.length);
        outputPort->postMessage(packet);
    }

    /*******************************************************************
     * start frame operation mode work:
     * The implementation was sufficiently different to separate.
     ******************************************************************/
    void startFrameModeWork(void)
    {
        auto inputPort = this->input(0);
        auto outputPort = this->output(0);

        //get input buffer
        auto inBuff = inputPort->buffer();
        if (inBuff.length == 0) return;

        bool frameFound = false;
        Pothos::Packet packet;

        auto inLen = inBuff.length;
        for (auto &label : inputPort->labels())
        {
            // Skip any label that doesn't yet appear in the data buffer
            if(label.index >= inLen)
                continue;

            // If the packet is not a start-of-frame packet, yet it is within the packet's data boundaries,
            // append that label to the packet message, with correction about the label position
            if(label.id != _frameStartId)
            {
                if(frameFound)
                {
                    if(label.index < _mtu)
                    {
                        packet.labels.push_back(label);
                    }
                }
                continue;
            }

            // Skip all of data before the start of packet if this is the first time we see the label
            auto dataStartIndex = label.index;
            if(dataStartIndex != 0 && !frameFound)
            {
                inputPort->consume(dataStartIndex);
                inputPort->setReserve(_mtu);
                return;
            }

            // This case happens when the start of frame is naturally aligned with the begining of a buffer, but we didn't get enough data
            // In that case we wait
            if(dataStartIndex == 0 && inLen < _mtu)
            {
                inputPort->setReserve(_mtu);
                return;
            }

            if(dataStartIndex == 0)
            {
                frameFound = true;
            }

            // If we see multiple frame labels, skip all the other ones for now
        }

        // Skip all of the data in case we didn't see any frame labels
        if(!frameFound)
        {
            inputPort->consume(inLen);
            return;
        }

        inBuff.length = _mtu;
        packet.payload = inBuff;
        outputPort->postMessage(packet);
        inputPort->consume(_mtu);
    }

    void propagateLabels(const Pothos::InputPort *)
    {
        //labels are not propagated
        return;
    }

    //the circular buffer allows stream to packet to forwards maximum sized payloads of available input buffer
    //its also useful for efficiency purposes when running in the start of frame only mode
    Pothos::BufferManager::Sptr getInputBufferManager(const std::string &, const std::string &)
    {
        return Pothos::BufferManager::make("circular");
    }

private:

    void updateModes(void)
    {
        _startFrameMode = not _frameStartId.empty() and _frameEndId.empty();
        _fullFrameMode = not _frameStartId.empty() and not _frameEndId.empty();
    }

    size_t _mtu;
    std::string _frameStartId;
    std::string _frameEndId;
    bool _inFrame;
    bool _startFrameMode;
    bool _fullFrameMode;
};

static Pothos::BlockRegistry registerStreamToPacket(
    "/blocks/stream_to_packet", &StreamToPacket::make);
