// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <cstring>
#include "MacHelper.hpp"

/***********************************************************************
 * |PothosDoc Simple MAC
 *
 * This MAC is a simple implementation of a media access control layer.
 * https://en.wikipedia.org/wiki/Media_access_control
 *
 * |category /Packet
 * |keywords MAC PHY packet
 *
 * |param macId A 16-bit address of the MAC interface
 * |default 0
 *
 * |factory /blocks/simple_mac()
 * |setter setMacId(macId)
 **********************************************************************/
class SimpleMac : public Pothos::Block
{
public:
    SimpleMac(void):
        _id(0),
        _errorCount(0)
    {
        this->setupInput("phyIn");
        this->setupInput("macIn");
        this->setupOutput("phyOut");
        this->setupOutput("macOut");
        this->registerCall(this, POTHOS_FCN_TUPLE(SimpleMac, setMacId));
    }

    static Block *make(void)
    {
        return new SimpleMac();
    }

    void activate(void)
    {
        _phyIn = this->input("phyIn");
        _macIn = this->input("macIn");
        _phyOut = this->output("phyOut");
        _macOut = this->output("macOut");
    }

    void setMacId(uint16_t macId)
    {
        _id = macId;
    }

    Pothos::BufferChunk unpack(const Pothos::Packet &pkt, uint16_t &senderId, uint16_t &recipientId)
    {
        const auto byteBuf = pkt.payload.as<const uint8_t *>();

        if (pkt.payload.length < 7) return Pothos::BufferChunk();

        // Data byte format: CRC SENDER_MSB SENDER_LSB RECIPIENT_MSB RECIPIENT_LSB LENGTH_MSB LENGTH_LSB
        int headerSize = 0;
        uint8_t crc = byteBuf[headerSize]; headerSize += 1;
        senderId = (byteBuf[headerSize] << 8) + byteBuf[headerSize + 1]; headerSize += 2;
        recipientId = (byteBuf[headerSize] << 8) + byteBuf[headerSize + 1]; headerSize += 2;
        uint16_t packetLength = (byteBuf[headerSize] << 8) + byteBuf[headerSize + 1]; headerSize += 2;

        // checking for the unfinished packet
        if (packetLength > pkt.payload.length) return Pothos::BufferChunk();

        if (recipientId != _id) return Pothos::BufferChunk();

        //check crc
        auto newCrc = Crc8(byteBuf + 1, packetLength - 1);
        if (newCrc != crc) return Pothos::BufferChunk();

        //return the payload
        auto payload = pkt.payload;
        payload.length = packetLength - headerSize;
        payload.address += headerSize;
    
        return payload;
    }

    void work(void)
    {
        //check phy input packets for crc and send to the mac out
        if (_phyIn->hasMessage())
        {
            auto msg = _phyIn->popMessage();
            auto pktIn = msg.extract<Pothos::Packet>();
            Pothos::Packet pktOut;
            uint16_t recipientId, senderId;
            pktOut.payload = this->unpack(pktIn, recipientId, senderId);
            if (pktOut.payload)
            {
                pktOut.metadata["recipient"] = Pothos::Object(recipientId);
                pktOut.metadata["sender"] = Pothos::Object(senderId);
                _macOut->postMessage(pktOut);
            }
            else
                _errorCount++;
        }

        //mac input packets are protocol framed and sent to the phy out
        if (_macIn->hasMessage())
        {
            auto msg = _macIn->popMessage();
            auto pktIn = msg.extract<Pothos::Packet>();
            auto data = pktIn.payload;

            auto recipientIdIter = pktIn.metadata.find("recipient");
            if (recipientIdIter == pktIn.metadata.end())
            {
                _errorCount++;
                return;
            }
            auto recipientId = recipientIdIter->second.extract<uint16_t>();
    
            auto packetLength = data.length + 7;
            Pothos::Packet packetOut;
            packetOut.payload = Pothos::BufferChunk("uint8", packetLength);
            auto byteBuf = packetOut.payload.as<uint8_t *>();

            // Data byte format: CRC SENDER_MSB SENDER_LSB RECIPIENT_MSB RECIPIENT_LSB LENGTH_MSB LENGTH_LSB
            byteBuf[1] = _id >> 8;
            byteBuf[2] = _id & 0xFF;
            byteBuf[3] = recipientId >> 8;
            byteBuf[4] = recipientId & 0xFF;
            byteBuf[5] = packetLength >> 8;
            byteBuf[6] = packetLength & 0xFF;
            std::memcpy(byteBuf + 7, data.as<const uint8_t*>(), data.length);
            byteBuf[0] = Crc8(byteBuf + 1, packetLength - 1);

            _phyOut->postMessage(packetOut);
        }
    }

private:
    size_t _id;
    unsigned long long _errorCount;
    Pothos::OutputPort *_phyOut;
    Pothos::OutputPort *_macOut;
    Pothos::InputPort *_phyIn;
    Pothos::InputPort *_macIn;
};

static Pothos::BlockRegistry registerSimpleMac(
    "/blocks/simple_mac", &SimpleMac::make);
