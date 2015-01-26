// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include "MacHelper.hpp"

/***********************************************************************
 * |PothosDoc Simple MAC
 *
 * This MAC is a simple implementation of a media access control layer.
 * https://en.wikipedia.org/wiki/Media_access_control
 *
 *
 *
 * |category /Packet
 * |keywords MAC PHY packet
 *
 * |factory /blocks/simple_mac()
 **********************************************************************/
class SimpleMac : public Pothos::Block
{
public:
    SimpleMac(void):
        _seqNo(0),
        _id(0),
        _errorCount(0)
    {
        this->setupInput("phyIn");
        this->setupInput("macIn");
        this->setupOutput("phyOut");
        this->setupOutput("macOut");
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

    Pothos::BufferChunk unpack(const Pothos::Packet &pkt)
    {
        const auto hdr = pkt.payload.as<const MacHeader *>();

        //short packet or bad length
        if (pkt.payload.length < hdr->bytes) return Pothos::BufferChunk();

        //check the id
        if (hdr->id != _id) return Pothos::BufferChunk();

        //check crc
        auto hdrCrc = hdr->crc;
        const_cast<MacHeader *>(hdr)->crc = 0;
        auto newCrc = Crc8(pkt.payload.as<const void *>(), hdr->bytes);
        if (newCrc != hdrCrc) return Pothos::BufferChunk();

        //return the payload
        auto payload = pkt.payload;
        payload.length = hdr->bytes - sizeof(MacHeader);
        payload.address += sizeof(MacHeader);
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
            pktOut.payload = this->unpack(pktIn);
            if (pktOut.payload) _macOut->postMessage(pktOut);
        }

        //mac input packets are protocol framed and sent to the phy out
        if (_macIn->hasMessage())
        {
            auto msg = _macIn->popMessage();
            auto pktIn = msg.extract<Pothos::Packet>();
        }
    }

private:
    size_t _seqNo;
    size_t _id;
    unsigned long long _errorCount;
    Pothos::OutputPort *_phyOut;
    Pothos::OutputPort *_macOut;
    Pothos::InputPort *_phyIn;
    Pothos::InputPort *_macIn;
};

static Pothos::BlockRegistry registerSimpleMac(
    "/blocks/simple_mac", &SimpleMac::make);
