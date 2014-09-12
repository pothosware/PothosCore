// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <iostream>

/***********************************************************************
 * |PothosDoc MessageToSignal
 *
 * The message to signal block takes an input message on port 0,
 * and emits a signal of the name specified in the configuration.
 * The signal prototype has one argument which is the message contents.
 *
 * |category /Utility
 * |keywords message signal
 *
 * |param name[Signal Name] The name of the signal to emit.
 * |default "itChanged"
 *
 * |factory /blocks/message_to_signal(name)
 **********************************************************************/
class MessageToSignal : public Pothos::Block
{
public:
    static Block *make(const std::string &name)
    {
        return new MessageToSignal(name);
    }

    MessageToSignal(const std::string &name):
        _emitName(name)
    {
        this->setupInput(0);
        this->registerSignal(name);
    }

    void work(void)
    {
        auto input = this->input(0);
        if (input->hasMessage())
        {
            this->callVoid(_emitName, input->popMessage());
        }
    }

private:
    const std::string _emitName;
};

static Pothos::BlockRegistry registerMessageToSignal(
    "/blocks/message_to_signal", &MessageToSignal::make);
