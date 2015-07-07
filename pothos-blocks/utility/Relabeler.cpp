// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <algorithm> //min/max

/***********************************************************************
 * |PothosDoc Relabeler
 *
 * The relabeler block passively forwards a stream of data
 * while applying the labels specified on another stream.
 * The primary input port 0 provides the data stream,
 * while the label input port "lbl" provides the labels.
 * The data and labels are forwarded to output port 0.
 *
 * <b> Note:</b>
 * Both input ports must be used in a synchronous manner,
 * with equal amounts of data fed to each input port.
 *
 * <b>Example usage:</b>
 * Suppose that a signal went through a mathematical transform,
 * and then was labeled to indicate meaningful events in the sample stream.
 * The user doesn't want to use the output of the mathematical transform,
 * instead they want the associated labels annotating the original signal.
 *
 * |category /Utility
 * |keywords stream label
 *
 * |param keepPrimary[Keep Primary] How to handle labels from the primary input port.
 * Labels will always be forwarded from the input labels port,
 * however the user may configure how labels from the primary port are handled.
 * |default false
 * |option [Forward] true
 * |option [Discard] false
 *
 * |factory /blocks/relabeler()
 * |setter setKeepPrimary(keepPrimary)
 **********************************************************************/
class Relabeler : public Pothos::Block
{
public:
    static Block *make(void)
    {
        return new Relabeler();
    }

    Relabeler(void):
        _lblPort(nullptr),
        _keepPrimary(false)
    {
        this->setupInput(0);
        this->setupInput("lbl");
        this->setupOutput(0, "", this->uid()); //unique domain because of buffer forwarding
        _lblPort = this->input("lbl");
        this->registerCall(this, POTHOS_FCN_TUPLE(Relabeler, setKeepPrimary));
        this->registerCall(this, POTHOS_FCN_TUPLE(Relabeler, getKeepPrimary));
    }

    void setKeepPrimary(const bool keep)
    {
        _keepPrimary = keep;
    }

    bool getKeepPrimary(void) const
    {
        return _keepPrimary;
    }

    void work(void)
    {
        auto inPort = this->input(0);
        auto outPort = this->output(0);

        while (inPort->hasMessage())
        {
            const auto msg = inPort->popMessage();
            outPort->postMessage(msg);
        }

        //we can only forward as many elements from primate as we have labels
        const size_t N = std::min(inPort->buffer().elements(), _lblPort->buffer().elements());
        if (N == 0) return;

        //grab the primary buffer and set the length
        auto buff = inPort->buffer();
        buff.length = N*buff.dtype.size();

        //consume and forward buffer
        inPort->consume(N*inPort->buffer().dtype.size());
        _lblPort->consume(N*_lblPort->buffer().dtype.size());
        outPort->postBuffer(buff);
    }

    void propagateLabels(const Pothos::InputPort *port)
    {
        //labels are discarded here from the primary port based on keep setting
        if (port == this->input(0) and not _keepPrimary) return;

        //propagate labels with default implementation
        Pothos::Block::propagateLabels(port);
    }

private:
    Pothos::InputPort *_lblPort;
    bool _keepPrimary;
};

static Pothos::BlockRegistry registerRelabeler(
    "/blocks/relabeler", &Relabeler::make);
