// Copyright (c) 2014-2014 Rinat Zakirov
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <list>
#include <algorithm>
#include <cstring>

/***********************************************************************
 * |PothosDoc LabelDeframer
 *
 * Extracts packets from a data stream based on a match label.
 *
 * |category /Packet
 *
 * |param packetLength The length of the packet as a number of elements
 * |default 1
 *
 * |param frameStartLabel The label that signifies the beginning of the frame
 * |default "Match!"
 *
 * |factory /blocks/labelframer()
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

    LabelDeframer(void) : packetLength(0x1), fillingPackets()
    {
        this->setupInput(0, typeid(unsigned char));
        this->setupOutput(0, typeid(unsigned char));
        this->registerCall(this, POTHOS_FCN_TUPLE(LabelDeframer, setPacketLength));
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

        auto inBytes = inBuff.as<const uint8_t*>();
        auto inLen = inBuff.elements();
        auto lastOldPacket = fillingPackets.rbegin();
        for (auto i = inputPort->labels().begin(), end = inputPort->labels().end(); i != end;)
        {
            auto &label = *i;
    
            // Skip any label that doesn't yet appear in the data buffer
            if(label.index >= inLen)
            {
                i++;
                continue;
            }

            // If the packet is not a start-of-frame packet, yet it is within the packet's data boundaries, 
            // append that label to the packet message, with correction about the label position
            if(label.id != frameStartLabel)
            {
                for(auto i = fillingPackets.begin(), end = fillingPackets.end(); i != end; i++)
                {
                    PacketSlot &slot = *i;
                    if(slot.startIndex == -1) // If the the packet was appended in the previous iterations of the work function
                    {
                        if(label.index < (packetLength - slot.fillLevel))
                        {
                            slot.labels.push_back(label);
                            slot.labels.back().index += slot.fillLevel;
                        }
                    } 
                }
                i = inputPort->labels().erase(i);
                continue;
            }
    
            // For the start-of-packet label, queue up a new packet with a buffer 
            auto dataStartIndex = label.index;

            fillingPackets.push_back(PacketSlot()); 
            PacketSlot &slot = fillingPackets.back(); 
            slot.packet.payload = Pothos::BufferChunk(packetLength);
             
            auto availableData = inLen - dataStartIndex;
            availableData = std::min(availableData, packetLength);
            std::memcpy(slot.packet.payload.as<uint8_t*>(), inBytes + dataStartIndex, availableData);
            slot.fillLevel = availableData; 
            slot.startIndex = dataStartIndex;

            i = inputPort->labels().erase(i);
        }

        for(auto i = lastOldPacket, rend = fillingPackets.rend(); i != rend;)
        {
            PacketSlot &slot = *i;

            int newFillLevel = slot.fillLevel + inLen;
            newFillLevel = std::min(newFillLevel, packetLength);

            if(newFillLevel != slot.fillLevel)
                std::memcpy(slot.packet.payload.as<uint8_t*>() + slot.fillLevel, inBytes, newFillLevel - slot.fillLevel);

            slot.fillLevel = newFillLevel;
            if(newFillLevel == packetLength)
            {
                outputPort->postMessage(slot.packet);
                PacketList::iterator toErase = i.base();
                i++;
                fillingPackets.erase(toErase);
            }
            else
                i++;
        }

        //produce/consume
        inputPort->consume(inLen);
    }

protected:

    struct PacketSlot
    {
        Pothos::Packet packet;
        int32_t startIndex;
        int32_t fillLevel;
    };

    typedef std::list<PacketSlot> PacketList;
    PacketList fillingPackets;
    int packetLength;
    std::string frameStartLabel;
};

static Pothos::BlockRegistry registerLabelDeframer(
    "/blocks/labeldeframer", &LabelDeframer::make);
