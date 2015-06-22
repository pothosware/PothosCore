// Copyright (c) 2015-2015 Rinat Zakirov
// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <algorithm> //min/max

/***********************************************************************
 * |PothosDoc Bits To Symbols
 *
 * Pack a stream of bits from input port 0 to a stream of symbols on output port 0.
 * Each input byte represents a bit and can take the values of 0 and 1.
 * Each output byte represents a symbol of bit width specified by modulus.
 *
 * This block also accepts packet messages on input port 0.
 * The payload will be converted and posted to output port 0.
 *
 * This block can be used to convert between bytes and bits when symbol size is 8.
 *
 * |category /Digital
 * |category /Symbol
 *
 * |param N[Modulus] The number of bits per symbol.
 * |default 2
 * |widget SpinBox(minimum=1, maximum=8)
 *
 * |param bitOrder[Bit Order] The bit ordering: MSBit or LSBit.
 * For MSBit, input 0 becomes the high bit of the output symbol.
 * For LSBit, input 0 becomes the low bit of the output symbol.
 * |option [MSBit] "MSBit"
 * |option [LSBit] "LSBit"
 * |default "MSBit"
 *
 * |factory /blocks/bits_to_symbols()
 * |setter setModulus(N)
 * |setter setBitOrder(bitOrder)
 **********************************************************************/
class BitsToSymbols : public Pothos::Block
{
public:

    static Block *make(void)
    {
        return new BitsToSymbols();
    }

    BitsToSymbols(void) : _order(BitOrder::MSBit), _mod(1)
    {
        this->setupInput(0, typeid(unsigned char));
        this->setupOutput(0, typeid(unsigned char));
        this->registerCall(this, POTHOS_FCN_TUPLE(BitsToSymbols, getModulus));
        this->registerCall(this, POTHOS_FCN_TUPLE(BitsToSymbols, setModulus));
        this->registerCall(this, POTHOS_FCN_TUPLE(BitsToSymbols, setBitOrder));
        this->registerCall(this, POTHOS_FCN_TUPLE(BitsToSymbols, getBitOrder));
    }

    unsigned char getModulus(void) const
    {
        return _mod;
    }

    void setModulus(const unsigned char mod)
    {
        if(mod>8)
        {
            throw Pothos::InvalidArgumentException("BitsToSymbols::setModulus()", "Modulus must be <= 8");
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
        else throw Pothos::InvalidArgumentException("BitsToSymbols::setBitOrder()", "Order must be LSBit or MSBit");
    }

    void bitsToSymbols(const unsigned char *in, unsigned char *out, const size_t len)
    {
        unsigned char sampleBit = 0x1;
        if (_order == BitOrder::LSBit) sampleBit = 1 << (_mod - 1);
        for (size_t i = 0; i < len; i++)
        {
            unsigned char symbol = 0;
            for (size_t b = 0; b < _mod; b++)
            {
                if(_order == BitOrder::MSBit)
                    symbol = symbol << 1;
                else
                    symbol = symbol >> 1;

                symbol |= (*in++ != 0) ? sampleBit : 0;
            }
            out[i] = symbol;
        }
    }

    void msgWork(void)
    {
        auto inputPort = this->input(0);
        auto outputPort = this->output(0);

        auto msg = inputPort->popMessage();
        if (msg.type() != typeid(Pothos::Packet))
        {
            outputPort->postMessage(msg);
            return;
        }

        //create a new packet for output symbols
        const auto &packet = msg.extract<Pothos::Packet>();
        Pothos::Packet newPacket;
        newPacket.payload = Pothos::BufferChunk("uint8", packet.payload.elements() / _mod);

        //perform conversion
        this->bitsToSymbols(
            packet.payload.as<const unsigned char*>(),
            newPacket.payload.as<unsigned char*>(),
            newPacket.payload.elements());

        //copy and adjust labels
        for (const auto &label : packet.labels)
        {
            newPacket.labels.push_back(label.toAdjusted(1, _mod));
        }

        //post the output packet
        outputPort->postMessage(newPacket);
    }

    void work(void)
    {
        auto inputPort = this->input(0);
        auto outputPort = this->output(0);
        inputPort->setReserve(_mod);

        //handle packet conversion if applicable
        if (inputPort->hasMessage()) this->msgWork();

        //setup buffers
        auto inBuff = inputPort->buffer();
        auto outBuff = outputPort->buffer();
        const size_t symLen = std::min(inBuff.elements() / _mod, outBuff.elements());
        if (symLen == 0) return;

        //perform conversion
        this->bitsToSymbols(
            inBuff.as<const unsigned char*>(),
            outBuff.as<unsigned char*>(),
            symLen);

        //produce/consume
        inputPort->consume(symLen * _mod);
        outputPort->produce(symLen);
    }

    void propagateLabels(const Pothos::InputPort *port)
    {
        auto outputPort = this->output(0);
        for (const auto &label : port->labels())
        {
            outputPort->postLabel(label.toAdjusted(1, _mod));
        }
    }

protected:
    typedef enum {LSBit, MSBit} BitOrder;
    BitOrder _order;
    unsigned char _mod;
};

static Pothos::BlockRegistry registerBitsToSymbols(
    "/blocks/bits_to_symbols", &BitsToSymbols::make);
