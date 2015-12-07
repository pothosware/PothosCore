// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>

/***********************************************************************
 * |PothosDoc Label To Message
 *
 * The label to message block listens for a label of the specified name, and
 * posts the label data to a message port.
 *
 * |category /Event
 * |category /Convert
 * |category /Labels
 * |keywords message label
 *
 * |param id[Label ID] The id of the label to respond to.
 * |default "test"
 *
 * |factory /blocks/label_to_message(id)
 **********************************************************************/
class LabelToMessage : public Pothos::Block
{
public:
    static Block *make(const std::string &id)
    {
        return new LabelToMessage(id);
    }

    LabelToMessage(const std::string &id):
        _labelId(id)
    {
        this->setupInput(0);
        this->setupOutput(0);

        this->input(0)->setReserve(1);
    }

    void work(void)
    {
        auto inPort = this->input(0);
        for (const auto &label : inPort->labels())
        {
            if (label.id == _labelId)
                this->output(0)->postMessage(label.data);
        }

        inPort->consume(inPort->elements());
    }

private:
    std::string const _labelId;
};

static Pothos::BlockRegistry registerLabelToMessage(
    "/blocks/label_to_message", &LabelToMessage::make);
