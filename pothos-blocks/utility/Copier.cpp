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

        auto elems = this->workInfo().minElements;
        if (elems == 0) return;
        std::memcpy(
            outputPort->buffer().as<void *>(),
            inputPort->buffer().as<const void *>(),
            elems*1);
        inputPort->consume(elems);
        outputPort->produce(elems);
    }
};

static Pothos::BlockRegistry registerCopier(
    "/blocks/copier", &Copier::make);
