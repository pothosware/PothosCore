// Copyright (c) 2015-2015 Rinat Zakirov
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>

/***********************************************************************
 * |PothosDoc SymbolsToBits
 *
 * Converts encoding symbols into data as a bit stream.
 *
 * |category /Digital
 * |category /Symbol
 *
 * |param symbols Number of possible symbols encoded in a byte. 
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
 * |param endianness Specify if bits are sent most or least significant bit first.
 * |default "MSB"
 * |option [MSB First] "MSB"
 * |option [LSB First] "LSB"
 *
 * |factory /blocks/symbols_to_bits()
 * |setter setSymbols(symbols)
 * |setter setEndianness(endianness)
 **********************************************************************/
class SymbolsToBits : public Pothos::Block
{
public:

    static Block *make(void)
    {
        return new SymbolsToBits();
    }

    SymbolsToBits(void) : msbFirst(true), symbolsMask(0x01), bitsPerSymbol(1)
    {
        this->setupInput(0, typeid(unsigned char));
        this->setupOutput(0, typeid(unsigned char));
        this->registerCall(this, POTHOS_FCN_TUPLE(SymbolsToBits, setEndianness));
        this->registerCall(this, POTHOS_FCN_TUPLE(SymbolsToBits, setSymbols));
    }

    void setSymbols(const size_t symbols)
    {
        if(symbols ==   2) { symbolsMask = 0x01; bitsPerSymbol = 1; }
        if(symbols ==   4) { symbolsMask = 0x03; bitsPerSymbol = 2; }
        if(symbols ==   8) { symbolsMask = 0x07; bitsPerSymbol = 3; }
        if(symbols ==  16) { symbolsMask = 0x0f; bitsPerSymbol = 4; }
        if(symbols ==  32) { symbolsMask = 0x1f; bitsPerSymbol = 5; }
        if(symbols ==  64) { symbolsMask = 0x3f; bitsPerSymbol = 6; }
        if(symbols == 128) { symbolsMask = 0x7f; bitsPerSymbol = 7; }
        if(symbols == 256) { symbolsMask = 0xff; bitsPerSymbol = 8; }
    }

    void setEndianness(const std::string &type)
    {
        msbFirst = true;
        if(type == "LSB") msbFirst = false;
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
        uint32_t symLen = std::min(inBuff.elements(), outBuff.elements() / bitsPerSymbol);

        auto inBytes = inBuff.as<const uint8_t*>();
        auto outBytes = outBuff.as<uint8_t*>();

        uint8_t sampleBit = 0x1;
        if(msbFirst) sampleBit = 1 << (bitsPerSymbol - 1);
        for(uint32_t i = 0; i < symLen; i++)
        {
            uint8_t symbol = *inBytes++;
            uint8_t mask = symbolsMask;
            while(mask)
            {
                mask = mask >> 1;   
                *outBytes++ = (sampleBit & symbol) != 0 ? 1 : 0;

                if(msbFirst)
                    symbol = symbol << 1; 
                else
                    symbol = symbol >> 1;
            }
        }

        //produce/consume
        inputPort->consume(symLen);
        outputPort->produce(symLen * bitsPerSymbol);
    }

protected:
    bool msbFirst;
    uint8_t symbolsMask;
    uint8_t bitsPerSymbol;
};

static Pothos::BlockRegistry registerSymbolsToBits(
    "/blocks/symbols_to_bits", &SymbolsToBits::make);
