// Copyright (c) 2014-2015 Rinat Zakirov
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>

/***********************************************************************
 * |PothosDoc BitsToSymbols
 *
 * Converts encoding symbols into data as a bit stream.
 *
 * |category /Digital
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
 * |factory /blocks/bitstosymbols()
 * |setter setSymbols(symbols)
 * |setter setEndianness(endianness)
 **********************************************************************/
class BitsToSymbols : public Pothos::Block
{
public:

    static Block *make(void)
    {
        return new BitsToSymbols();
    }

    BitsToSymbols(void) : msbFirst(true), symbolsMask(0x01), bitsPerSymbol(1)
    {
        this->setupInput(0, typeid(unsigned char));
        this->setupOutput(0, typeid(unsigned char));
        this->registerCall(this, POTHOS_FCN_TUPLE(BitsToSymbols, setEndianness));
        this->registerCall(this, POTHOS_FCN_TUPLE(BitsToSymbols, setSymbols));
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
        if (inBuff.length != 0)
        {        

            //setup output buffer
            auto outBuff = outputPort->buffer();
            uint32_t symLen = std::min(inBuff.elements() / bitsPerSymbol, outBuff.elements());

            auto inBytes = inBuff.as<const uint8_t*>();
            auto outBytes = outBuff.as<uint8_t*>();

            uint8_t sampleBit = 0x1;
            if(!msbFirst) sampleBit = 1 << (bitsPerSymbol - 1);
            for(uint32_t i = 0; i < symLen; i++)
            {
                uint8_t symbol = 0;
                uint8_t mask = symbolsMask;
                while(mask)
                {
                    mask = mask >> 1;   
                    if(msbFirst)
                        symbol = symbol << 1; 
                    else
                        symbol = symbol >> 1;

                    symbol |= (*inBytes++ != 0) ? sampleBit : 0; 
                }
                *outBytes++ = symbol;
            }

            //produce/consume
            inputPort->consume(symLen * bitsPerSymbol);
            outputPort->produce(symLen);
        }

        // Below code handles message based conversion

        if (not inputPort->hasMessage()) return;
        auto msg = inputPort->popMessage();
        
        if (msg.type() != typeid(Pothos::Packet))
        {
            outputPort->postMessage(msg);
            return;
        }

        const auto &packet = msg.extract<Pothos::Packet>();
        Pothos::Packet newPacket;
        auto symLen = packet.payload.elements() / bitsPerSymbol;
        newPacket.payload = Pothos::BufferChunk("uint8", symLen);
        
        auto inBytes = packet.payload.as<const uint8_t*>();
        auto outBytes = newPacket.payload.as<uint8_t*>();

        uint8_t sampleBit = 0x1;
        if(!msbFirst) sampleBit = 1 << (bitsPerSymbol - 1);

        for(uint32_t i = 0; i < symLen; i++)
        {
            uint8_t symbol = 0;
            uint8_t mask = symbolsMask;
            while(mask)
            {
                mask = mask >> 1;   
                if(msbFirst)
                    symbol = symbol << 1; 
                else
                    symbol = symbol >> 1;

                symbol |= (*inBytes++ != 0) ? sampleBit : 0; 
            }
            *outBytes++ = symbol;
        }

        outputPort->postMessage(newPacket);
    }

protected:
    bool msbFirst;
    uint8_t symbolsMask;
    uint8_t bitsPerSymbol;
};

static Pothos::BlockRegistry registerBitsToSymbols(
    "/blocks/bitstosymbols", &BitsToSymbols::make);
