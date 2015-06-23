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
 * The Preamble Framer parses a symbol stream on input port 0
 * for a special label indicating the start of a new frame.
 * The preamble is inserted into the stream before the label,
 * and the rest of the stream is forwarded to output port 0.
 *
 * This block supports operations on arbitrary symbol widths,
 * and therefore it may be used operationally on a bit-stream,
 * because a bit-stream is identically a symbol stream of N=1.
 *
 * |category /Digital
 * |keywords bit symbol preamble frame
 *
 * |param preamble A vector of symbols representing the preamble.
 * The width of each preamble symbol must the intended input stream.
 * |default [1]
 *
 * |param label The label id that marks the first symbol of frame data.
 * |default "Matched!"
 * |widget StringEntry()
 *
 * |factory /blocks/preamble_framer()
 * |setter setPreamble(preamble)
 * |setter setLabel(label)
 **********************************************************************/
class PreambleFramer : public Pothos::Block
{
public:
    static Block *make(void)
    {
        return new PreambleFramer();
    }

    PreambleFramer(void)
    {
        this->setupInput(0);
        this->setupOutput(0, typeid(unsigned char), this->uid()); //unique domain because of buffer forwarding
        this->registerCall(this, POTHOS_FCN_TUPLE(PreambleFramer, setPreamble));
        this->registerCall(this, POTHOS_FCN_TUPLE(PreambleFramer, getPreamble));
        this->registerCall(this, POTHOS_FCN_TUPLE(PreambleFramer, setLabel));
        this->registerCall(this, POTHOS_FCN_TUPLE(PreambleFramer, getLabel));
        this->setPreamble(std::vector<unsigned char>(1, 1)); //initial update
    }

    void setPreamble(const std::vector<unsigned char> preamble)
    {
        if (preamble.empty()) throw Pothos::InvalidArgumentException("PreambleFramer::setPreamble()", "preamble cannot be empty");
        _preamble = preamble;
        _preambleBuff = Pothos::BufferChunk((preamble.size()+7)/8);
    }

    std::vector<unsigned char> getPreamble(void) const
    {
        return _preamble;
    }

    void setLabel(std::string label)
    {
        _label = label;
    }

    std::string getLabel(void) const
    {
        return _label;
    }

    void work(void)
    {
    }

private:
    std::string _label;
    std::vector<unsigned char> _preamble;
    Pothos::BufferChunk _preambleBuff;
};

/***********************************************************************
 * registration
 **********************************************************************/
static Pothos::BlockRegistry registerPreambleFramer(
    "/blocks/preamble_framer", &PreambleFramer::make);
