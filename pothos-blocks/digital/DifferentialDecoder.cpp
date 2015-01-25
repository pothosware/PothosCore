// Copyright (c) 2014-2014 Rinat Zakirov
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>

/***********************************************************************
 * |PothosDoc DifferentialDecoder
 *
 * Implements the decoding part of: http://en.wikipedia.org/wiki/Differential_coding
 *
 * |category /Digital
 *
 * |param symbols 
 * |default 2
 * |option 2
 * |option 4
 * |option 8
 * |option 16
 * |option 32
 * |option 64
 * |option 128
 * |option 256
 *
 * |factory /blocks/differentialdecoder()
 * |setter setSymbols(symbols)
 **********************************************************************/
class DifferentialDecoder : public Pothos::Block
{
public:

    static Block *make(void)
    {
        return new DifferentialDecoder();
    }

    DifferentialDecoder(void) : lastSymRecv(0), symbolsMask(0x1)
    {
        this->setupInput(0, typeid(unsigned char));
        this->setupOutput(0, typeid(unsigned char));
        this->registerCall(this, POTHOS_FCN_TUPLE(DifferentialDecoder, setSymbols));
    }

    void setSymbols(const size_t symbols)
    {
        if(symbols ==   2) symbolsMask = 0x01;
        if(symbols ==   4) symbolsMask = 0x03;
        if(symbols ==   8) symbolsMask = 0x07;
        if(symbols ==  16) symbolsMask = 0x0f;
        if(symbols ==  32) symbolsMask = 0x1f;
        if(symbols ==  64) symbolsMask = 0x3f;
        if(symbols == 128) symbolsMask = 0x7f;
        if(symbols == 256) symbolsMask = 0xff;
    }

    void work(void)
    {
        auto inputPort = this->input(0);
        auto outputPort = this->output(0);

        //get input buffer
        auto inBuff = inputPort->buffer();
        if (inBuff.length == 0) return;

        //setup output buffer
        auto outBuff = outputPort->buffer();
        uint32_t len = std::min(inBuff.elements(), outBuff.elements());

        auto inBytes = inBuff.as<const uint8_t*>();
        auto outBytes = outBuff.as<uint8_t*>();

        uint8_t lastRecv = lastSymRecv;
        for(uint32_t i = 0; i < len; i++)
        {
            uint8_t last = lastRecv;
            lastRecv = *inBytes++;
            *outBytes++ = (last ^ lastRecv) & symbolsMask; 
        }
        lastSymRecv = lastRecv;

        //produce/consume
        inputPort->consume(len);
        outputPort->produce(len);
    }

protected:
    uint8_t lastSymRecv;
    uint8_t symbolsMask;
};

static Pothos::BlockRegistry registerDifferentialDecoder(
    "/blocks/differentialdecoder", &DifferentialDecoder::make);
