// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <cstring> //memset
#include <cassert>
#include <iostream>

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
 * |param dtype[Data Type] The datatype this block consumes.
 * |preview disable
 * |default "float32"
 *
 * |factory /blocks/misc/forwarder(dtype)
 **********************************************************************/
class Forwarder : public Pothos::Block
{
public:
    static Block *make(const Pothos::DType &dtype)
    {
        //TODO we shouldnt need to specify a dtype
        return new Forwarder(dtype);
    }

    Forwarder(const Pothos::DType &dtype)
    {
        this->setupInput(0, dtype);
        this->setupOutput(0, dtype);
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
