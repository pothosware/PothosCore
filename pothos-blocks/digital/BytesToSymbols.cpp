// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <cstdint>
#include <iostream>
#include <complex>
#include <vector>

/***********************************************************************
 * |PothosDoc Bytes to Symbols
 *
 * Unpack an incoming stream of bytes to N-bit symbols.
 *
 * |category /Digital
 * |category /Symbol
 * |keywords pack bit byte symbol chunk
 *
 * |param N[Modulus] The number of bits per symbol.
 * |default 2
 *
 * |param bitOrder[Bit Order] The bit ordering: MSBit or LSBit.
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

    BytesToSymbols(void)
    {
        _mod = 1;
        _mask = 1;
        _nb = 0;
        _rem = 0;
        _order = BitOrder::LSBit;
        this->setupInput(0, typeid(unsigned char));
        this->setupOutput(0, typeid(unsigned char));
        this->registerCall(this, POTHOS_FCN_TUPLE(BytesToSymbols, getModulus));
        this->registerCall(this, POTHOS_FCN_TUPLE(BytesToSymbols, setModulus));
        this->registerCall(this, POTHOS_FCN_TUPLE(BytesToSymbols, setBitOrder));
    }

    unsigned char getModulus(void) const
    {
        return _mod;
    }

    void updateMask(void)
    {
        if(_order == BitOrder::MSBit)
        {
            _mask = ((1<<_mod) - 1) << (8*sizeof(unsigned int)-_mod);
        } else {
            _mask = (1<<_mod) - 1;
        }
//        std::cout << "New mask: " << std::hex << int(_mask) << std::dec << std::endl;
    }

    void setModulus(const unsigned char mod)
    {
        if(mod>8)
        {
            throw Pothos::InvalidArgumentException("BytesToSymbols::setModulus()", "Modulus must be <= 8");
        }
        _mod = mod;
        updateMask();
    }

    std::string getBitOrder(void)
    {
        if(_order == BitOrder::LSBit) return "LSBit";
        else return "MSBit";
    }

    void setBitOrder(std::string order)
    {
        if(order == "LSBit") _order = BitOrder::LSBit;
        else if(order == "MSBit") _order = BitOrder::MSBit;
        else throw Pothos::InvalidArgumentException("BytesToSymbols::setBitOrder()", "Order must be LSBit or MSBit");
        updateMask();
    }

    void work(void)
    {
        auto inPort = this->input(0);
        auto outPort = this->output(0);

        auto in = inPort->buffer().template as<const unsigned char *>();
        auto out = outPort->buffer().template as<unsigned char *>();

        int N = inPort->elements();
        int M = outPort->elements();
        int n = 0;
        int m = 0;

        if(_order == BitOrder::LSBit)
        {
            while(n<N && m<M)
            {
                _rem |= (in[n++] << _nb);
                _nb += 8;
                while(_nb >= _mod)
                {
                    out[m] = 0;
                    for(int i=0; i<_mod; i++) {
                        out[m] <<= 1;
                        out[m] |= _rem & 1;
                        _rem >>= 1;
                    }
                    m++;
                    _nb -= _mod;
                }
            }
        } else {
            const int shift = 8*(sizeof(unsigned int));
            while(n<N && m<M)
            {
                _rem |= (in[n++]<<(shift-8-_nb));
                _nb += 8;
                while(_nb >= _mod)
                {
                    out[m++] = (_rem & _mask) >> (shift-_mod);
                    _rem <<= _mod;
                    _nb -= _mod;
                }
            }
        }

        inPort->consume(n);
        outPort->produce(m);
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
static Pothos::BlockRegistry registerBytesToSymbols(
    "/blocks/bytes_to_symbols", &BytesToSymbols::make);

