// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "SymbolHelpers.hpp"
#include <Pothos/Framework.hpp>
#include <cstdint>
#include <iostream>
#include <complex>
#include <vector>

/***********************************************************************
 * |PothosDoc Symbols to Bytes
 *
 * Pack an incoming stream of N-bit symbols into bytes.
 * Each input byte represents a symbol of bit width specified by modulus.
 *
 * This block also accepts packet messages on input port 0.
 * The payload will be converted and posted to output port 0.
 *
 * |category /Digital
 * |category /Symbol
 * |keywords pack bit byte symbol chunk
 *
 * |param N[Modulus] The number of bits per symbol.
 * |default 2
 * |widget SpinBox(minimum=1, maximum=8)
 *
 * |param bitOrder[Bit Order] The bit ordering: MSBit or LSBit.
 * For MSBit, input symbols get packed high to low in the output byte.
 * For LSBit, input symbols get packed low to high in the output byte.
 * |option [MSBit] "MSBit"
 * |option [LSBit] "LSBit"
 * |default "MSBit"
 *
 * |factory /blocks/symbols_to_bytes()
 * |setter setModulus(N)
 * |setter setBitOrder(bitOrder)
 **********************************************************************/
class SymbolsToBytes : public Pothos::Block
{
public:
    static Block *make(void)
    {
        return new SymbolsToBytes();
    }

    SymbolsToBytes(void):
        _mod(1),
        _order(BitOrder::LSBit)
    {
        this->setupInput(0, typeid(unsigned char));
        this->setupOutput(0, typeid(unsigned char));
        this->registerCall(this, POTHOS_FCN_TUPLE(SymbolsToBytes, getModulus));
        this->registerCall(this, POTHOS_FCN_TUPLE(SymbolsToBytes, setModulus));
        this->registerCall(this, POTHOS_FCN_TUPLE(SymbolsToBytes, setBitOrder));
        this->registerCall(this, POTHOS_FCN_TUPLE(SymbolsToBytes, getBitOrder));
    }

    unsigned char getModulus(void) const
    {
        return _mod;
    }

    void setModulus(const unsigned char mod)
    {
        if (mod < 1 or mod > 8)
        {
            throw Pothos::InvalidArgumentException("SymbolsToBytes::setModulus()", "Modulus must be between 1 and 8 inclusive");
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
        else throw Pothos::InvalidArgumentException("SymbolsToBytes::setBitOrder()", "Order must be LSBit or MSBit");
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

        //create a new packet for output bytes
        const auto &packet = msg.extract<Pothos::Packet>();
        const size_t numBytes = (packet.payload.elements()*_mod)/8;
        Pothos::Packet newPacket;
        newPacket.payload = Pothos::BufferChunk(numBytes);

        //perform conversion
        auto in = packet.payload.as<const unsigned char*>();
        auto out = newPacket.payload.as<unsigned char*>();
        switch (_order)
        {
        case MSBit: ::symbolsToBytesMSBit(_mod, in, out, numBytes); break;
        case LSBit: ::symbolsToBytesLSBit(_mod, in, out, numBytes); break;
        }

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
        auto inPort = this->input(0);
        auto outPort = this->output(0);

        //handle packet conversion if applicable
        if (inPort->hasMessage()) this->msgWork();

        //calculate work size -- based on having multiples of 8 bits
        const size_t numBytes = std::min((inPort->elements()*_mod)/8, outPort->elements());
        if (numBytes == 0) return;

        //perform the conversion to pack each output byte
        auto in = inPort->buffer().as<const unsigned char *>();
        auto out = outPort->buffer().as<unsigned char *>();
        switch (_order)
        {
        case MSBit: ::symbolsToBytesMSBit(_mod, in, out, numBytes); break;
        case LSBit: ::symbolsToBytesLSBit(_mod, in, out, numBytes); break;
        }

        //consume input symbols and output bytes
        inPort->consume((numBytes*8)/_mod);
        outPort->produce(numBytes);
    }

    void propagateLabels(const Pothos::InputPort *port)
    {
        auto outputPort = this->output(0);
        for (const auto &label : port->labels())
        {
            outputPort->postLabel(label.toAdjusted(_mod, 8));
        }
    }

private:
    unsigned char _mod;
    BitOrder _order;
};

/***********************************************************************
 * registration
 **********************************************************************/
static Pothos::BlockRegistry registerSymbolsToBytes(
    "/blocks/symbols_to_bytes", &SymbolsToBytes::make);

