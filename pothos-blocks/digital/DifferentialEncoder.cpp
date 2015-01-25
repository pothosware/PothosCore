// Copyright (c) 2014-2015 Rinat Zakirov
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>

/***********************************************************************
 * |PothosDoc DifferentialEncoder
 *
 * Implements the encoding part of: http://en.wikipedia.org/wiki/Differential_coding
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
 * |factory /blocks/differentialencoder()
 * |setter setSymbols(symbols)
 **********************************************************************/
class DifferentialEncoder : public Pothos::Block
{
public:

    static Block *make(void)
    {
        return new DifferentialEncoder();
    }

    DifferentialEncoder(void) : lastSymSent(0), symbolsMask(0x1)
    {
        this->setupInput(0, typeid(unsigned char));
        this->setupOutput(0, typeid(unsigned char));
        this->registerCall(this, POTHOS_FCN_TUPLE(DifferentialEncoder, setSymbols));
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

        uint8_t lastSent = lastSymSent;
        for(uint32_t i = 0; i < len; i++)
        {
            lastSent = (*inBytes++ ^ lastSent) & symbolsMask; 
            *outBytes++ = lastSent;
        }
        lastSymSent = lastSent;

        //produce/consume
        inputPort->consume(len);
        outputPort->produce(len);
    }

protected:
    uint8_t lastSymSent;
    uint8_t symbolsMask;
};

static Pothos::BlockRegistry registerDifferentialEncoder(
    "/blocks/differentialencoder", &DifferentialEncoder::make);
