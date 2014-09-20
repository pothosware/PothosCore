// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "SerializeCommon.hpp"
#include <Pothos/Framework.hpp>
#include <Poco/ByteOrder.h>
#include <sstream>
#include <cstring>
#include <cassert>

/***********************************************************************
 * |PothosDoc Serializer
 *
 * Serialize the contents of 1 or more input ports into a mVRL stream.
 * The mVRL stream encapsulates input streams, labels, and messages.
 * The streaming data is left in its original binary format.
 * The contents of the labels and messages are serialized.
 * The input ports are indexed starting at 0 and incrementing.
 * The serializer outputs a byte stream containing mVRL on port 0.
 *
 * The modified VITA radio link layer (mVRL) protocol encapsulates packets,
 * preserving both the packet order and the packet boundaries.
 * A VRL stream can be stored to file, or sent across a network.
 * In the event of loss, the bounds of the stream can be recovered.
 *
 * |category /Serialize
 * |keywords serialize VRL
 *
 * |factory /blocks/serializer()
 **********************************************************************/
class Serializer : public Pothos::Block
{
public:
    Serializer(void)
    {
        this->setupInput(0);
        this->setupOutput(0);
    }

    static Block *make(void)
    {
        return new Serializer();
    }

    void work(void);

    void activate(void)
    {
        _seqs.resize(this->inputs().size());
    }

private:
    std::vector<size_t> _seqs;
};

static Pothos::BlockRegistry registerSerializer(
    "/blocks/serializer", &Serializer::make);

/*!
 * Pack header fields into an outgoing buffer.
 */
static void packBuffer(const size_t seq, const size_t sid, const bool has_tsf, const unsigned long long tsf, const bool is_ext, Pothos::BufferChunk &buff)
{
    assert(buff.length > 0);
    const size_t hdr_words32 = has_tsf? 6 : 4;
    const size_t pkt_bytes = hdr_words32*4 + buff.length + 4;
    const size_t pkt_words32 = hdr_words32 + padUp32(buff.length)/4 + 1;
    const size_t vita_words32 = pkt_words32 - 3;

    //adjust address/length for full packet
    assert(buff.address >= hdr_words32*4);
    buff.address -= hdr_words32*4;
    buff.length = pkt_words32*4;

    auto p = buff.as<Poco::UInt32 *>();
    p[0] = Poco::ByteOrder::toNetwork(mVRL);
    p[1] = Poco::ByteOrder::toNetwork(Poco::UInt32(((seq << 20) & 0xfff) | (pkt_bytes & 0xfffff)));
    p[2] = Poco::ByteOrder::toNetwork(Poco::UInt32(VITA_SID | (is_ext? VITA_EXT : 0) | (has_tsf? VITA_TSF : 0) | ((seq << 16) & 0xf) | (vita_words32 & 0xffff)));
    p[3] = Poco::ByteOrder::toNetwork(Poco::UInt32(sid));
    if (has_tsf) p[4] = Poco::ByteOrder::toNetwork(Poco::UInt32(tsf >> 32));
    if (has_tsf) p[5] = Poco::ByteOrder::toNetwork(Poco::UInt32(tsf >> 0));
    p[pkt_words32-1] = Poco::ByteOrder::toNetwork(VEND);
}

/*!
 * Serialize an object into a buffer.
 */
static Pothos::BufferChunk objectToOffsetBuffer(const size_t offset_words32, const Pothos::Object &obj)
{
    //serialize the object into a string
    std::stringstream ss;
    obj.serialize(ss);

    //memcpy the stringstream into a buffer
    Pothos::BufferChunk buff(padUp32(ss.str().length()) + HDR_TLR_BYTES); //string length + padding
    buff.length = ss.str().length();
    buff.address += offset_words32*4;
    std::memcpy(buff.as<void *>(), ss.str().data(), buff.length);
    return buff;
}

void Serializer::work(void)
{
    auto outputPort = this->output(0);

    for (size_t i = 0; i < this->inputs().size(); i++)
    {
        auto inputPort = this->input(i);

        //messages (async messages handled asap)
        while (inputPort->hasMessage())
        {
            auto msg = inputPort->popMessage();
            const size_t hdr_words32 = 4; // a priori
            auto buff = objectToOffsetBuffer(hdr_words32, msg);
            packBuffer(_seqs[i]++, i, false, 0, true, buff);
            outputPort->postBuffer(buff);
        }

        //labels (always handled prior to buffers for ordering reasons)
        while (inputPort->labels().begin() != inputPort->labels().end())
        {
            auto lbl = *inputPort->labels().begin();
            inputPort->removeLabel(lbl);
            const size_t hdr_words32 = 6; // a priori
            auto buff = objectToOffsetBuffer(hdr_words32, Pothos::Object(lbl));
            auto index = lbl.index + inputPort->totalElements();
            packBuffer(_seqs[i]++, i, true, index, true, buff);
            outputPort->postBuffer(buff);
        }

        //buffers
        auto buff = inputPort->buffer();
        if (buff.length > 0)
        {
            auto hdrTlrBuff = Pothos::BufferChunk(buff.length+HDR_TLR_BYTES);
            const size_t hdr_words32 = 6; // a priori
            hdrTlrBuff.address += hdr_words32*4;
            hdrTlrBuff.length = buff.length;
            packBuffer(_seqs[i]++, i, true, inputPort->totalElements(), false, hdrTlrBuff);

            //post the header
            hdrTlrBuff.length = hdr_words32*4;
            outputPort->postBuffer(hdrTlrBuff);

            //post the payload
            outputPort->postBuffer(buff);
            inputPort->consume(buff.length);

            //post the trailer
            const size_t pkt_words32 = hdr_words32 + padUp32(buff.length)/4 + 1;
            hdrTlrBuff.address += (pkt_words32 - 1)*4;
            hdrTlrBuff.length = 4;
            outputPort->postBuffer(hdrTlrBuff);
        }
    }
}
