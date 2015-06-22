// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

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
 * |category /Digital
 * |category /Symbol
 * |keywords pack bit byte symbol chunk
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
        _mask(1),
        _rem(0),
        _nb(0),
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

    void updateMask(void)
    {
        if (_order == BitOrder::MSBit)
        {
            _mask = ((1<<_mod) - 1) << (8*sizeof(unsigned int)-_mod);
        }
        else
        {
            _mask = (1<<_mod) - 1;
        }
    }

    void setModulus(const unsigned char mod)
    {
        if(mod>8)
        {
            throw Pothos::InvalidArgumentException("SymbolsToBytes::setModulus()", "Modulus must be <= 8");
        }
        _mod = mod;
        this->updateMask();
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
        this->updateMask();
    }

    void work(void)
    {
        auto inPort = this->input(0);
        auto outPort = this->output(0);

        auto in = inPort->buffer().template as<const unsigned char *>();
        auto out = outPort->buffer().template as<unsigned char *>();

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
    unsigned int _mask;
    unsigned int _rem;
    unsigned char _nb;
    typedef enum {LSBit, MSBit} BitOrder;
    BitOrder _order;
};

/***********************************************************************
 * registration
 **********************************************************************/
static Pothos::BlockRegistry registerSymbolsToBytes(
    "/blocks/symbols_to_bytes", &SymbolsToBytes::make);

