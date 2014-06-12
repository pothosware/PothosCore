// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>

/***********************************************************************
 * |PothosDoc Forwarder
 *
 * The forwarder block passivly forwards all data from
 * input port 0 to the output port 0 without copying.
 * This block is mainly used for testing purposes.
 *
 * |category /Misc
 * |keywords forwarder
 *
 * |factory /blocks/misc/forwarder()
 **********************************************************************/
class Forwarder : public Pothos::Block
{
public:
    static Block *make(void)
    {
        return new Forwarder();
    }

    Forwarder(void)
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

        const auto &buffer = inputPort->buffer();
        if (buffer.length != 0)
        {
            outputPort->postBuffer(buffer);
            inputPort->consume(inputPort->elements());
        }
    }
};

static Pothos::BlockRegistry registerForwarder(
    "/blocks/misc/forwarder", &Forwarder::make);
