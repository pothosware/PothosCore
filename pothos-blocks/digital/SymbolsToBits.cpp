// Copyright (c) 2015-2015 Rinat Zakirov
// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "SymbolHelpers.hpp"
#include <Pothos/Framework.hpp>
#include <algorithm> //min/max

/***********************************************************************
 * |PothosDoc Symbols To Bits
 *
 * Unpack a stream of symbols from input port 0 to a stream of bits on output port 0.
 * Each input byte represents a symbol of bit width specified by modulus.
 * Each output byte represents a bit and can take the values of 0 and 1.
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
 * For MSBit, the high bit of the input symbol becomes output 0.
 * For LSBit, the low bit of the input symbol becomes output 0.
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
        if (mod < 1 or mod > 8)
        {
            throw Pothos::InvalidArgumentException("SymbolsToBits::setModulus()", "Modulus must be between 1 and 8 inclusive");
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

    void msgWork(const Pothos::Packet &inPkt)
    {
        //calculate conversion and buffer sizes
        const size_t numSyms = inPkt.payload.length;
        const size_t numBits = numSyms*_mod;

        //create a new packet for output symbols
        Pothos::Packet outPkt;
        auto outPort = this->output(0);
        if (outPort->elements() >= numBits)
        {
            outPkt.payload = outPort->buffer();
            outPkt.payload.length = numBits;
            outPort->popBuffer(numBits);
        }
        else outPkt.payload = Pothos::BufferChunk(outPort->dtype(), numBits);

        //perform conversion
        auto in = inPkt.payload.as<const unsigned char*>();
        auto out = outPkt.payload.as<unsigned char*>();
        switch (_order)
        {
        case MSBit: ::symbolsToBitsMSBit(_mod, in, out, numSyms); break;
        case LSBit: ::symbolsToBitsLSBit(_mod, in, out, numSyms); break;
        }

        //copy and adjust labels
        for (const auto &label : inPkt.labels)
        {
            outPkt.labels.push_back(label.toAdjusted(_mod, 1));
        }

        //post the output packet
        outPort->postMessage(outPkt);
    }

    void work(void)
    {
        auto inPort = this->input(0);
        auto outPort = this->output(0);

        //handle packet conversion if applicable
        if (inPort->hasMessage())
        {
            auto msg = inPort->popMessage();
            if (msg.type() == typeid(Pothos::Packet))
                this->msgWork(msg.extract<Pothos::Packet>());
            else outPort->postMessage(msg);
            return; //output buffer used, return now
        }

        //calculate work size
        const size_t numSyms = std::min(inPort->elements(), outPort->elements() / _mod);
        if (numSyms == 0) return;

        //perform conversion
        auto in = inPort->buffer().as<const unsigned char *>();
        auto out = outPort->buffer().as<unsigned char *>();
        switch (_order)
        {
        case MSBit: ::symbolsToBitsMSBit(_mod, in, out, numSyms); break;
        case LSBit: ::symbolsToBitsLSBit(_mod, in, out, numSyms); break;
        }

        //produce/consume
        inPort->consume(numSyms);
        outPort->produce(numSyms * _mod);
    }

    void propagateLabels(const Pothos::InputPort *port)
    {
        auto outputPort = this->output(0);
        for (const auto &label : port->labels())
        {
            outputPort->postLabel(label.toAdjusted(_mod, 1));
        }
    }

protected:
    BitOrder _order;
    unsigned char _mod;
};

static Pothos::BlockRegistry registerSymbolsToBits(
    "/blocks/symbols_to_bits", &SymbolsToBits::make);
