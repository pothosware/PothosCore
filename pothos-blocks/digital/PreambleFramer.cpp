// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <cstdint>
#include <complex>
#include <cassert>
#include <iostream>

/***********************************************************************
 * |PothosDoc Preamble Framer
 *
 * The Preamble Framer accepts input packet messages on port 0
 * and forwards each packet into an output byte stream domain
 * led by a preamble of packed bytes specified by the user.
 *
 * |category /Digital
 * |keywords bit preamble frame
 *
 * |param preamble The unpacked vector of bits representing preamble to match.
 * |default [1]
 *
 * |param endianness Specify if bits are sent most or least significant bit first.
 * |default "MSB"
 * |option [MSB First] "MSB"
 * |option [LSB First] "LSB"
 *
 * |factory /blocks/preamble_framer()
 * |setter setPreamble(preamble)
 * |setter setEndianness(endianness)
 **********************************************************************/
class PreambleFramer : public Pothos::Block
{
public:
    static Block *make(void)
    {
        return new PreambleFramer();
    }

    PreambleFramer(void):
        _msbFirst(true)
    {
        this->setupInput(0);
        this->setupOutput(0, typeid(unsigned char), this->uid()); //unique domain because of buffer forwarding
        this->registerCall(this, POTHOS_FCN_TUPLE(PreambleFramer, setPreamble));
        this->registerCall(this, POTHOS_FCN_TUPLE(PreambleFramer, getPreamble));
        this->registerCall(this, POTHOS_FCN_TUPLE(PreambleFramer, setEndianness));
        this->setPreamble(std::vector<unsigned char>(1, 1)); //initial update
    }

    void setPreamble(const std::vector<unsigned char> preamble)
    {
        if (preamble.empty()) throw Pothos::InvalidArgumentException("PreambleFramer::setPreamble()", "preamble cannot be empty");
        _preamble = preamble;
        this->updatePreableBytes();
    }

    std::vector<unsigned char> getPreamble(void) const
    {
        return _preamble;
    }

    void setEndianness(const std::string &type)
    {
        _msbFirst = type == "MSB";
        this->updatePreableBytes();
    }

    void work(void)
    {
    }

    void updatePreableBytes(void)
    {
        
        //_preambleBytes = Pothos::BufferChunk((preamble.size()+7)/8);

    }

private:
    bool _msbFirst;
    std::vector<unsigned char> _preamble;
    Pothos::BufferChunk _preambleBytes;
};

/***********************************************************************
 * registration
 **********************************************************************/
static Pothos::BlockRegistry registerPreambleFramer(
    "/blocks/preamble_framer", &PreambleFramer::make);
