// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <cstring>
#include "MacHelper.hpp"

/***********************************************************************
 * |PothosDoc Simple MAC
 *
 * The Simple MAC block is an implementation of a media access control layer.
 * It acts as an interface between physical-layer datagrams and a layer of addressable packets.
 * Packets from the MAC to the PHY get a packet header to embed source, destination and checksum.
 * Packets from the PHY to the MAC have their header inspected and stripped from the payload.
 *
 * https://en.wikipedia.org/wiki/Media_access_control
 *
 * <h3>Error recovery</h3>
 * When the simple MAC detects a packet checksum error, it simply drops the packet.
 * The MAC block does not handle packet loss, error recovery, or resending of data.
 * However, the simple LLC block can be used with the simple MAC to implement reliability.
 *
 * <h2>Interfaces</h2>
 * The Simple MAC block has 4 ports that operate on packet streams:
 * <ul>
 * <li><b>macIn</b> - This port accepts a packet of user data
 *  where the metadata has the "recipient" field set to the remote destination MAC.</li>
 * <li><b>phyOut</b> - This port produces a packet of user data with an additional header
 *  containing source address, destination address, and checksum.</li>
 * <li><b>phyIn</b> - This port accepts a packet of physical-layer data,
 *  and inspects the payload for the MAC-layer header.
 *  Packets with checksum errors or incorrect destinations are dropped.</li>
 * <li><b>macOut</b> - This port produces a packet of user data
 *  where the metadata has the "sender" field set to the remote destination MAC.</li>
 * </ul>
 *
 * |category /Packet
 * |keywords MAC PHY packet
 *
 * |param macId[MAC ID] A 16-bit address of the MAC interface.
 * This address is used as the sender ID for outgoing PHY packets,
 * and is used to check the recipient ID for incoming PHY packets.
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
        this->registerCall(this, POTHOS_FCN_TUPLE(SimpleMac, getMacId));
        this->registerCall(this, POTHOS_FCN_TUPLE(SimpleMac, getErrorCount));
        this->registerProbe("getErrorCount");
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

    uint16_t getMacId(void) const
    {
        return _id;
    }

    unsigned long long getErrorCount(void) const
    {
        return _errorCount;
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
            Pothos::Packet pktOut = pktIn;
            uint16_t recipientId = 0, senderId = 0;
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
            auto recipientId = recipientIdIter->second.convert<uint16_t>();
    
            auto packetLength = data.length + 7;
            Pothos::Packet pktOut = pktIn;
            pktOut.payload = Pothos::BufferChunk(packetLength);
            pktOut.payload.dtype = pktIn.payload.dtype;
            auto byteBuf = pktOut.payload.as<uint8_t *>();

            // Data byte format: CRC SENDER_MSB SENDER_LSB RECIPIENT_MSB RECIPIENT_LSB LENGTH_MSB LENGTH_LSB
            byteBuf[1] = _id >> 8;
            byteBuf[2] = _id & 0xFF;
            byteBuf[3] = recipientId >> 8;
            byteBuf[4] = recipientId & 0xFF;
            byteBuf[5] = packetLength >> 8;
            byteBuf[6] = packetLength & 0xFF;
            std::memcpy(byteBuf + 7, data.as<const uint8_t*>(), data.length);
            byteBuf[0] = Crc8(byteBuf + 1, packetLength - 1);

            _phyOut->postMessage(pktOut);
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
