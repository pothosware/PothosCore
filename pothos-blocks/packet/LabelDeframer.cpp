// Copyright (c) 2015-2015 Rinat Zakirov
// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <algorithm>

/***********************************************************************
 * |PothosDoc Label Deframer
 *
 * Extracts packets from a data stream based on a match label.
 *
 * |category /Packet
 *
 * |param mtu[MTU] The length of the each output packet as a number of elements.
 * |default 1024
 *
 * |param frameStartId[Frame Start ID] The label ID that signifies the beginning of the frame.
 * |default "frameStart"
 * |widget StringEntry()
 *
 * |factory /blocks/label_deframer()
 * |setter setMTU(mtu)
 * |setter setFrameStartId(frameStartId)
 **********************************************************************/
class LabelDeframer : public Pothos::Block
{
public:

    static Block *make(void)
    {
        return new LabelDeframer();
    }

    LabelDeframer(void) : _mtu(1024), _frameStartId("frameStart")
    {
        this->setupInput(0, typeid(unsigned char));
        this->setupOutput(0, typeid(unsigned char));
        this->registerCall(this, POTHOS_FCN_TUPLE(LabelDeframer, setMTU));
        this->registerCall(this, POTHOS_FCN_TUPLE(LabelDeframer, getMTU));
        this->registerCall(this, POTHOS_FCN_TUPLE(LabelDeframer, setFrameStartId));
        this->registerCall(this, POTHOS_FCN_TUPLE(LabelDeframer, getFrameStartId));
    }

    void setMTU(const size_t mtu)
    {
        _mtu = mtu;
    }

    size_t getMTU(void) const
    {
        return _mtu;
    }

    void setFrameStartId(const std::string &id)
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

        //get input buffer
        auto inBuff = inputPort->buffer();
        if (inBuff.length == 0) return;

        bool frameFound = false;
        Pothos::Packet packet;

        auto inLen = inBuff.elements() * inBuff.dtype.size();;
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

    Pothos::BufferManager::Sptr getInputBufferManager(const std::string &, const std::string &)
    {
        return Pothos::BufferManager::make("circular");
    }

private:
    size_t _mtu;
    std::string _frameStartId;
};
/*
static Pothos::BlockRegistry registerLabelDeframer(
    "/blocks/label_deframer", &LabelDeframer::make);
*/
