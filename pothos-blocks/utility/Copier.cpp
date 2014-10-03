// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <cstring> //memcpy

/***********************************************************************
 * |PothosDoc Copier
 *
 * The copier block copies all data from input port 0 to the output port 0.
 * This block is used to bridge connections between incompatible domains.
 *
 * |category /Utility
 *
 * |factory /blocks/copier()
 **********************************************************************/
class Copier : public Pothos::Block
{
public:
    static Block *make(void)
    {
        return new Copier();
    }

    Copier(void)
    {
        this->setupInput(0);
        this->setupOutput(0);
    }

    void work(void)
    {
        auto inputPort = this->input(0);
        auto outputPort = this->output(0);

        if (inputPort->hasMessage())
        {
            auto m = inputPort->popMessage();
            outputPort->postMessage(m);
        }

        //get input buffer
        auto inBuff = inputPort->buffer();
        if (inBuff.length == 0) return;

        //setup output buffer
        auto outBuff = outputPort->buffer();
        outBuff.dtype = inBuff.dtype;
        outBuff.length = std::min(inBuff.elements(), outBuff.elements())*outBuff.dtype.size();

        //copy input to output
        std::memcpy(outBuff.as<void *>(), inBuff.as<const void *>(), outBuff.length);

        //produce/consume
        inputPort->consume(outBuff.length);
        outputPort->popBuffer(outBuff.length);
        outputPort->postBuffer(outBuff);
    }
};

static Pothos::BlockRegistry registerCopier(
    "/blocks/copier", &Copier::make);
