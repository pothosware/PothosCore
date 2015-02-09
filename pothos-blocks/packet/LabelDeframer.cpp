// Copyright (c) 2015-2015 Rinat Zakirov
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <list>
#include <algorithm>
#include <cstring>

/***********************************************************************
 * |PothosDoc Simplified Label Deframer
 *
 * Extracts packets from a data stream based on a match label.
 *
 * |category /Packet
 *
 * |param packetLength The length of the packet as a number of elements
 * |default 1
 *
 * |param frameStartLabel The label that signifies the beginning of the frame
 * |default "Matched!"
 * |widget StringEntry()
 *
 * |factory /blocks/label_deframer()
 * |setter setPacketLength(packetLength)
 * |setter setFrameStartLabel(frameStartLabel)
 **********************************************************************/
class LabelDeframer : public Pothos::Block
{
public:

    static Block *make(void)
    {
        return new LabelDeframer();
    }

    LabelDeframer(void) : packetLength(0x1), frameStartLabel("Matched!")
    {
        this->setupInput(0, typeid(unsigned char));
        this->setupOutput(0, typeid(unsigned char));
        this->registerCall(this, POTHOS_FCN_TUPLE(LabelDeframer, setPacketLength));
        this->registerCall(this, POTHOS_FCN_TUPLE(LabelDeframer, setFrameStartLabel));
    }

    void setFrameStartLabel(const std::string &label)
    {
        frameStartLabel = label;
    }

    void setPacketLength(const size_t length)
    {
        packetLength = length;
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
            if(label.id != frameStartLabel)
            {
                if(frameFound)
                {
                    if(label.index < packetLength)
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
                inputPort->setReserve(packetLength);
                return;
            } 

            // This case happens when the start of frame is naturally aligned with the begining of a buffer, but we didn't get enough data
            // In that case we wait
            if(dataStartIndex == 0 && inLen < packetLength)
            {
                inputPort->setReserve(packetLength);
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

        inBuff.length = packetLength;
        packet.payload = inBuff;
        outputPort->postMessage(packet);
        inputPort->consume(packetLength);
    }

    Pothos::BufferManager::Sptr getInputBufferManager(const std::string &, const std::string &)
    {
        return Pothos::BufferManager::make("circular");
    }

protected:

    size_t packetLength;
    std::string frameStartLabel;
};

static Pothos::BlockRegistry registerLabelDeframer(
    "/blocks/label_deframer", &LabelDeframer::make);
