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
 * |param preambleLength
 * |default 0
 * 
 * |param packetLength
 * |default 1
 *
 * |factory /blocks/labelframer()
 * |setter setPreambleLength(preambleLength)
 * |setter setPacketLength(packetLength)
 **********************************************************************/
class LabelDeframer : public Pothos::Block
{
public:

    static Block *make(void)
    {
        return new LabelDeframer();
    }

    LabelDeframer(void) : preambleLength(0), packetLength(0x1), fillingPackets()
    {
        this->setupInput(0, typeid(unsigned char));
        this->setupOutput(0, typeid(unsigned char));
        this->registerCall(this, POTHOS_FCN_TUPLE(LabelDeframer, setPreambleLength));
        this->registerCall(this, POTHOS_FCN_TUPLE(LabelDeframer, setPacketLength));
    }

    void setPreambleLength(const size_t length)
    {
        preambleLength = length; 
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
        PacketList::reverse_iterator lastOldPacket = fillingPackets.rbegin();
        for (const auto &label : inputPort->labels())
        {
            auto dataStartIndex = label.index + preambleLength;

            fillingPackets.push_back(PacketSlot()); 
            PacketSlot &slot = fillingPackets.back(); 
            slot.packet.payload = Pothos::BufferChunk(packetLength);
            
            if(dataStartIndex >= inLen)
                slot.fillLevel = inLen - dataStartIndex;            
            else
            {
                auto availableData = inLen - dataStartIndex;
                std::memcpy(slot.packet.payload.as<uint8_t*>(), inBytes + dataStartIndex, availableData);
                slot.fillLevel = availableData; 
            }
        }

        for(PacketList::reverse_iterator i = lastOldPacket, rend = fillingPackets.rend(); i != rend;)
        {
            PacketSlot &slot = *i;

            int newFillLevel = slot.fillLevel + inLen;
            newFillLevel = std::min(newFillLevel, packetLength);

            if(slot.fillLevel <= -1 && newFillLevel > 0)
                std::memcpy(slot.packet.payload.as<uint8_t*>(), inBytes - slot.fillLevel, newFillLevel);
            else if(slot.fillLevel >= 0 && newFillLevel != slot.fillLevel)
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
        int32_t fillLevel;
    };

    typedef std::list<PacketSlot> PacketList;
    PacketList fillingPackets;
    int preambleLength;
    int packetLength;
};

static Pothos::BlockRegistry registerLabelDeframer(
    "/blocks/labeldeframer", &LabelDeframer::make);
