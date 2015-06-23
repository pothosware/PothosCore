// Copyright (c) 2015-2015 Rinat Zakirov
// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "SymbolHelpers.hpp"
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
        if (mod < 1 or mod > 8)
        {
            throw Pothos::InvalidArgumentException("BitsToSymbols::setModulus()", "Modulus must be between 1 and 8 inclusive");
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

    void msgWork(void)
    {
        auto inPort = this->input(0);
        auto outPort = this->output(0);

        auto msg = inPort->popMessage();
        if (msg.type() != typeid(Pothos::Packet))
        {
            outPort->postMessage(msg);
            return;
        }

        //create a new packet for output symbols
        const auto &packet = msg.extract<Pothos::Packet>();
        const size_t numInBits = ((packet.payload.elements() + _mod - 1)/_mod)*_mod;
        const size_t numSyms = numInBits/_mod;
        Pothos::Packet newPacket;
        newPacket.payload = Pothos::BufferChunk(numSyms);

        //perform conversion
        auto in = packet.payload.as<const unsigned char*>();
        auto out = newPacket.payload.as<unsigned char*>();
        switch (_order)
        {
        case MSBit: ::bitsToSymbolsMSBit(_mod, in, out, numSyms); break;
        case LSBit: ::bitsToSymbolsLSBit(_mod, in, out, numSyms); break;
        }

        //copy and adjust labels
        for (const auto &label : packet.labels)
        {
            newPacket.labels.push_back(label.toAdjusted(1, _mod));
        }

        //post the output packet
        outPort->postMessage(newPacket);
    }

    void work(void)
    {
        auto inPort = this->input(0);
        auto outPort = this->output(0);
        inPort->setReserve(_mod);

        //handle packet conversion if applicable
        if (inPort->hasMessage()) this->msgWork();

        //calculate work size
        const size_t numSyms = std::min(inPort->elements() / _mod, outPort->elements());
        if (numSyms == 0) return;

        //perform conversion
        auto in = inPort->buffer().as<const unsigned char *>();
        auto out = outPort->buffer().as<unsigned char *>();
        switch (_order)
        {
        case MSBit: ::bitsToSymbolsMSBit(_mod, in, out, numSyms); break;
        case LSBit: ::bitsToSymbolsLSBit(_mod, in, out, numSyms); break;
        }

        //produce/consume
        inPort->consume(numSyms * _mod);
        outPort->produce(numSyms);
    }

    void propagateLabels(const Pothos::InputPort *port)
    {
        auto outPort = this->output(0);
        for (const auto &label : port->labels())
        {
            outPort->postLabel(label.toAdjusted(1, _mod));
        }
    }

protected:
    BitOrder _order;
    unsigned char _mod;
};

static Pothos::BlockRegistry registerBitsToSymbols(
    "/blocks/bits_to_symbols", &BitsToSymbols::make);
