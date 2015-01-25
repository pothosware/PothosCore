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
 * |param symbols Number of possible symbols encoded in a byte. 
 * |default 2
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

    DifferentialDecoder(void) : lastSymRecv(0), symbols(2)
    {
        this->setupInput(0, typeid(unsigned char));
        this->setupOutput(0, typeid(unsigned char));
        this->registerCall(this, POTHOS_FCN_TUPLE(DifferentialDecoder, setSymbols));
    }

    void setSymbols(const size_t symbols)
    {
        this->symbols = symbols;
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
            *outBytes++ = (lastRecv - last + symbols) % symbols;
        }
        lastSymRecv = lastRecv;

        //produce/consume
        inputPort->consume(len);
        outputPort->produce(len);
    }

protected:
    uint8_t lastSymRecv;
    uint32_t symbols;
};

static Pothos::BlockRegistry registerDifferentialDecoder(
    "/blocks/differentialdecoder", &DifferentialDecoder::make);
