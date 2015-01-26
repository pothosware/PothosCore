// Copyright (c) 2014-2014 Rinat Zakirov
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <list>
#include <algorithm>
#include <cstring>

/***********************************************************************
 * |PothosDoc Label Deframer
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
 * |factory /blocks/labeldeframer()
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

        auto inBytes = inBuff.as<const uint8_t*>();
        auto inLen = inBuff.elements();
        for (auto &label : inputPort->labels())
        {
            // Skip any label that doesn't yet appear in the data buffer
            if(label.index >= inLen)
                continue;

            // If the packet is not a start-of-frame packet, yet it is within the packet's data boundaries, 
            // append that label to the packet message, with correction about the label position
            if(label.id != frameStartLabel)
            {
                for(auto i = fillingPackets.begin(), end = fillingPackets.end(); i != end; i++)
                {
                    PacketSlot &slot = *i;
                    if(!slot.freshPacket) // If the the packet was appended in the previous iterations of the work function
                    {
                        if(label.index < (packetLength - slot.fillLevel))
                        {
                            slot.packet.labels.push_back(label);
                            slot.packet.labels.back().index += slot.fillLevel;
                        }
                    } 
                    else // otherwise(if the packet was appended during this work iteration)
                    {
                        if(label.index >= slot.startIndex && label.index < slot.startIndex + packetLength)
                        {
                            slot.packet.labels.push_back(label);
                            slot.packet.labels.back().index -= slot.startIndex;
                        }
                    }
                }
                continue;
            }
    
            // For the start-of-packet label, queue up a new packet with a buffer 
            auto dataStartIndex = label.index;

            fillingPackets.push_back(PacketSlot()); 
            PacketSlot &slot = fillingPackets.back(); 
            slot.packet.payload = Pothos::BufferChunk("uint8", packetLength);
             
            size_t availableData = inLen - dataStartIndex;
            availableData = std::min(availableData, packetLength);
            std::memcpy(slot.packet.payload.as<uint8_t*>(), inBytes + dataStartIndex, availableData);
            slot.fillLevel = availableData; 
            slot.startIndex = dataStartIndex;
            slot.freshPacket = true;
        }


        for(auto &slot : fillingPackets)
        {
            if(slot.freshPacket) continue;

            size_t newFillLevel = slot.fillLevel + inLen;
            newFillLevel = std::min(newFillLevel, packetLength);

            if(newFillLevel != slot.fillLevel)
                std::memcpy(slot.packet.payload.as<uint8_t*>() + slot.fillLevel, inBytes, newFillLevel - slot.fillLevel);


            slot.fillLevel = newFillLevel;
        }

        for(auto i = fillingPackets.begin(), end = fillingPackets.end(); i != end;)
        {
            PacketSlot &slot = *i;
            slot.freshPacket = false;

            if(slot.fillLevel == packetLength)
            {
                outputPort->postMessage(slot.packet);
                i = fillingPackets.erase(i);
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
        bool freshPacket;
        size_t startIndex;
        size_t fillLevel;
    };

    size_t packetLength;
    typedef std::list<PacketSlot> PacketList;
    PacketList fillingPackets;
    std::string frameStartLabel;
};

static Pothos::BlockRegistry registerLabelDeframer(
    "/blocks/labeldeframer", &LabelDeframer::make);
