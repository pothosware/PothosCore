// Copyright (c) 2015-2015 Rinat Zakirov
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <algorithm> //min/max

/***********************************************************************
 * |PothosDoc Symbols To Bits
 *
 * Unpack a stream of symbols from input port 0 to a stream of bits on output port 0.
 * Each input byte represents a symbol of bit width specified by modulus.
 * Each output byte represents a bit and can take the values of 0 and 1.
 *
 * |category /Digital
 * |category /Symbol
 *
 * |param N[Modulus] The number of bits per symbol.
 * |default 2
 * |widget SpinBox(minimum=1, maximum=8)
 *
 * |param bitOrder[Bit Order] The bit ordering: MSBit or LSBit.
 * |option [MSBit] "MSBit"
 * |option [LSBit] "LSBit"
 * |default "MSBit"
 *
 * |factory /blocks/symbols_to_bits()
 * |setter setModulus(N)
 * |setter setBitOrder(bitOrder)
 **********************************************************************/
class SymbolsToBits : public Pothos::Block
{
public:

    static Block *make(void)
    {
        return new SymbolsToBits();
    }

    SymbolsToBits(void) : _order(BitOrder::MSBit), _mod(1)
    {
        this->setupInput(0, typeid(unsigned char));
        this->setupOutput(0, typeid(unsigned char));
        this->registerCall(this, POTHOS_FCN_TUPLE(SymbolsToBits, getModulus));
        this->registerCall(this, POTHOS_FCN_TUPLE(SymbolsToBits, setModulus));
        this->registerCall(this, POTHOS_FCN_TUPLE(SymbolsToBits, setBitOrder));
        this->registerCall(this, POTHOS_FCN_TUPLE(SymbolsToBits, getBitOrder));
    }

    unsigned char getModulus(void) const
    {
        return _mod;
    }

    void setModulus(const unsigned char mod)
    {
        if(mod>8)
        {
            throw Pothos::InvalidArgumentException("SymbolsToBits::setModulus()", "Modulus must be <= 8");
        }
        _mod = mod;
    }

    std::string getBitOrder(void) const
    {
        return (_order == BitOrder::LSBit)? "LSBit" : "MSBit";
    }

    void setBitOrder(std::string order)
    {
        if (order == "LSBit") _order = BitOrder::LSBit;
        else if (order == "MSBit") _order = BitOrder::MSBit;
        else throw Pothos::InvalidArgumentException("SymbolsToBits::setBitOrder()", "Order must be LSBit or MSBit");
    }

    void symbolsToBits(const uint8_t *in, uint8_t *out, const size_t len)
    {
        uint8_t sampleBit = 0x1;
        if (_order == BitOrder::MSBit) sampleBit = 1 << (_mod - 1);
        for (size_t i = 0; i < len; i++)
        {
            uint8_t symbol = in[i];
            for (size_t b = 0; b < _mod; b++)
            {
                *out++ = ((sampleBit & symbol) != 0) ? 1 : 0;

                if(_order == BitOrder::MSBit)
                    symbol = symbol << 1;
                else
                    symbol = symbol >> 1;
            }
        }
    }

    void work(void)
    {
        auto inputPort = this->input(0);
        auto outputPort = this->output(0);

        //setup buffers
        auto inBuff = inputPort->buffer();
        auto outBuff = outputPort->buffer();
        const size_t symLen = std::min(inBuff.elements(), outBuff.elements() / _mod);
        if (symLen == 0) return;

        //perform conversion
        this->symbolsToBits(
            inBuff.as<const uint8_t*>(),
            outBuff.as<uint8_t*>(),
            symLen
        );

        //produce/consume
        inputPort->consume(symLen);
        outputPort->produce(symLen * _mod);
    }

protected:
    typedef enum {LSBit, MSBit} BitOrder;
    BitOrder _order;
    uint8_t _mod;
};

static Pothos::BlockRegistry registerSymbolsToBits(
    "/blocks/symbols_to_bits", &SymbolsToBits::make);
