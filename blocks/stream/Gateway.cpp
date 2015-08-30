// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>

/***********************************************************************
 * |PothosDoc Gateway
 *
 * The gateway block can forward, drop, or backup an input stream.
 * This block forwards input port 0 to the output port 0 without copying.
 * This block is mainly used for testing and debug purposes.
 *
 * |category /Stream
 * |keywords forward drop back pressure
 *
 * |param mode The handling mode for input data on port 0.
 * Messages, labels, and stream buffers are all subject to the forwarding mode.
 * |option [Forward] "FORWARD"
 * |option [Backup] "BACKUP"
 * |option [Drop] "DROP"
 *
 * |factory /blocks/gateway()
 * |setter setMode(mode)
 **********************************************************************/
class Gateway : public Pothos::Block
{
public:
    static Block *make(void)
    {
        return new Gateway();
    }

    Gateway(void):
        _forwardMode(false),
        _backupMode(false),
        _dropMode(false)
    {
        this->setupInput(0);
        this->setupOutput(0, "", this->uid()); //unique domain because of buffer forwarding);
        this->registerCall(this, POTHOS_FCN_TUPLE(Gateway, setMode));
        this->registerCall(this, POTHOS_FCN_TUPLE(Gateway, getMode));
        this->setMode("FORWARD");
    }

    void setMode(const std::string &mode)
    {
        _mode = _mode;
        _forwardMode = false;
        _backupMode = false;
        _dropMode = false;
        if (mode == "FORWARD") _forwardMode = true;
        if (mode == "BACKUP") _backupMode = true;
        if (mode == "DROP") _dropMode = true;
    }

    std::string getMode(void) const
    {
        return _mode;
    }

    void work(void)
    {
        auto inputPort = this->input(0);
        auto outputPort = this->output(0);

        //backup mode? just return, dont consume
        if (_backupMode) return;

        //drop mode? consume all input, return
        if (_dropMode)
        {
            while (inputPort->hasMessage())
            {
                inputPort->popMessage();
            }
            while (inputPort->labels().begin() != inputPort->labels().end())
            {
                const auto &label = *inputPort->labels().begin();
                inputPort->removeLabel(label);
            }
            inputPort->consume(inputPort->elements());
            return;
        }

        //forward mode - forward everything
        if (not _forwardMode) return;

        while (inputPort->hasMessage())
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

private:
    std::string _mode;
    bool _forwardMode;
    bool _backupMode;
    bool _dropMode;
};

static Pothos::BlockRegistry registerGateway(
    "/blocks/gateway", &Gateway::make);
