// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <iostream>

/***********************************************************************
 * |PothosDoc Slot To Message
 *
 * The slot to message block handles a slot of the specified name,
 * and posts the first argument of the slot to a message port port.
 *
 * |category /Event
 * |category /Convert
 * |keywords message slot
 *
 * |param name[Signal Name] The name of the slot to accept signals on.
 * |default "handleIt"
 *
 * |factory /blocks/slot_to_message(name)
 **********************************************************************/
class SlotToMessage : public Pothos::Block
{
public:
    static Block *make(const std::string &name)
    {
        return new SlotToMessage(name);
    }

    SlotToMessage(const std::string &name):
        _slotName(name)
    {
        this->setupOutput(0);
        this->registerSlot(name); //see opaqueCallHandler comment below
    }

    //Pothos is cool because you can have advanced overload hooks like this,
    //but dont use this block as a good example of signal and slots usage.
    Pothos::Object opaqueCallHandler(const std::string &name, const Pothos::Object *inputArgs, const size_t numArgs)
    {
        if (name == _slotName)
        {
            if (numArgs > 0) this->output(0)->postMessage(inputArgs[0]);
            return Pothos::Object();
        }
        return Pothos::Block::opaqueCallHandler(name, inputArgs, numArgs);
    }

private:
    const std::string _slotName;
};

static Pothos::BlockRegistry registerSlotToMessage(
    "/blocks/slot_to_message", &SlotToMessage::make);
