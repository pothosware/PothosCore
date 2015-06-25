// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "SymbolHelpers.hpp"
#include <Pothos/Framework.hpp>
#include <algorithm> //min/max
#include <iostream>

/***********************************************************************
 * |PothosDoc Bytes to Symbols
 *
 * Unpack an incoming stream of bytes into N-bit symbols.
 * Each output byte represents a symbol of bit width specified by modulus.
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
 * For MSBit, input bytes get unpacked high to low into output symbols.
 * For LSBit, input bytes get unpacked low to high into output symbols.
 * |option [MSBit] "MSBit"
 * |option [LSBit] "LSBit"
 * |default "MSBit"
 *
 * |factory /blocks/bytes_to_symbols()
 * |setter setModulus(N)
 * |setter setBitOrder(bitOrder)
 **********************************************************************/
class BytesToSymbols : public Pothos::Block
{
public:
    static Block *make(void)
    {
        return new BytesToSymbols();
    }

    BytesToSymbols(void):
        _mod(1),
        _reserveBytes(1),
        _order(BitOrder::LSBit)
    {
        this->setupInput(0, typeid(unsigned char));
        this->setupOutput(0, typeid(unsigned char));
        this->registerCall(this, POTHOS_FCN_TUPLE(BytesToSymbols, getModulus));
        this->registerCall(this, POTHOS_FCN_TUPLE(BytesToSymbols, setModulus));
        this->registerCall(this, POTHOS_FCN_TUPLE(BytesToSymbols, setBitOrder));
        this->registerCall(this, POTHOS_FCN_TUPLE(BytesToSymbols, getBitOrder));
    }

    unsigned char getModulus(void) const
    {
        return _mod;
    }

    void setModulus(const unsigned char mod)
    {
        if (mod < 1 or mod > 8)
        {
            throw Pothos::InvalidArgumentException("BytesToSymbols::setModulus()", "Modulus must be between 1 and 8 inclusive");
        }
        _mod = mod;

        switch (_mod)
        {
        case 7: _reserveBytes = 7; break;
        case 5: _reserveBytes = 5; break;
        case 3: _reserveBytes = 3; break;
        case 6: _reserveBytes = 3; break;
        default: _reserveBytes = 1; break;
        }
    }

    std::string getBitOrder(void) const
    {
        return (_order == BitOrder::LSBit)? "LSBit" : "MSBit";
    }

    void setBitOrder(std::string order)
    {
        if (order == "LSBit") _order = BitOrder::LSBit;
        else if (order == "MSBit") _order = BitOrder::MSBit;
        else throw Pothos::InvalidArgumentException("BytesToSymbols::setBitOrder()", "Order must be LSBit or MSBit");
    }

    void msgWork(const Pothos::Packet &inPkt)
    {
        //calculate conversion and buffer sizes (round up)
        const size_t numBytes = ((inPkt.payload.elements() + _reserveBytes - 1)/_reserveBytes)*_reserveBytes;
        const size_t numSyms = (numBytes*8)/_mod;

        //create a new packet for output symbols
        Pothos::Packet outPkt;
        auto outPort = this->output(0);
        if (outPort->elements() >= numSyms)
        {
            outPkt.payload = outPort->buffer();
            outPkt.payload.length = numSyms;
            outPort->popBuffer(numSyms);
        }
        else outPkt.payload = Pothos::BufferChunk(outPort->dtype(), numSyms);

        //perform conversion
        auto in = inPkt.payload.as<const unsigned char*>();
        auto out = outPkt.payload.as<unsigned char*>();
        switch (_order)
        {
        case MSBit: ::bytesToSymbolsMSBit(_mod, in, out, numBytes); break;
        case LSBit: ::bytesToSymbolsLSBit(_mod, in, out, numBytes); break;
        }

        //copy and adjust labels
        for (const auto &label : inPkt.labels)
        {
            outPkt.labels.push_back(label.toAdjusted(8, _mod));
        }

        //post the output packet
        outPort->postMessage(outPkt);
    }

    void work(void)
    {
        auto inPort = this->input(0);
        auto outPort = this->output(0);
        inPort->setReserve(_reserveBytes);

        //handle packet conversion if applicable
        if (inPort->hasMessage())
        {
            auto msg = inPort->popMessage();
            if (msg.type() == typeid(Pothos::Packet))
                this->msgWork(msg.extract<Pothos::Packet>());
            else outPort->postMessage(msg);
            return; //output buffer used, return now
        }

        //calculate work size given reserve requirements
        const size_t numInBytes = (inPort->elements()/_reserveBytes)*_reserveBytes;
        const size_t reserveSyms = (_reserveBytes*8)/_mod;
        const size_t numOutSyms = (outPort->elements()/reserveSyms)*reserveSyms;
        const size_t numBytes = std::min((numOutSyms*_mod)/8, numInBytes);
        if (numBytes == 0) return;

        //perform conversion
        auto in = inPort->buffer().as<const unsigned char *>();
        auto out = outPort->buffer().as<unsigned char *>();
        switch (_order)
        {
        case MSBit: ::bytesToSymbolsMSBit(_mod, in, out, numBytes); break;
        case LSBit: ::bytesToSymbolsLSBit(_mod, in, out, numBytes); break;
        }

        //consume input bytes and output symbols
        inPort->consume(numBytes);
        outPort->produce((numBytes*8)/_mod);
    }

    void propagateLabels(const Pothos::InputPort *port)
    {
        auto outputPort = this->output(0);
        for (const auto &label : port->labels())
        {
            outputPort->postLabel(label.toAdjusted(8, _mod));
        }
    }

private:
    unsigned char _mod;
    size_t _reserveBytes;
    BitOrder _order;
};

/***********************************************************************
 * registration
 **********************************************************************/
static Pothos::BlockRegistry registerBytesToSymbols(
    "/blocks/bytes_to_symbols", &BytesToSymbols::make);

