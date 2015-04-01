// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <iostream>
#include <algorithm> //min/max

/***********************************************************************
 * |PothosDoc Channel Aligner
 *
 * The channel aligner block consumes N input streams and uses timestamps
 * and selective dropping to output N time-aligned output streams.
 *
 * <ul>
 * <li>This is a zero-copy block, buffers are passively forwarded to the output.</li>
 * <li>This is a zero-conf block, sample rate is passed via inline stream labels.</li>
 * <li>This type agnostic block, data type is determined from the input buffers.</li>
 * </ul>
 *
 * |category /SDR
 * |keywords time align channel mimo
 *
 * |param numChans[Num Channels] The number of channels.
 * |default 2
 * |widget SpinBox(minimum=2)
 * |preview disable
 *
 * |factory /sdr/channel_aligner()
 * |initializer setNumChannels(numChans)
 **********************************************************************/
class ChannelAligner : public Pothos::Block
{
public:
    static Block *make(void)
    {
        return new ChannelAligner();
    }

    ChannelAligner(void):
        _sampleRate(1.0),
        _droppedSamps(0)
    {
        this->setupInput(0);
        this->setupOutput(0, "", this->uid()); //unique domain because of buffer forwarding
        this->registerCall(this, POTHOS_FCN_TUPLE(ChannelAligner, setNumChannels));
        this->registerCall(this, POTHOS_FCN_TUPLE(ChannelAligner, dropped));
        this->registerProbe("dropped");
    }

    void setNumChannels(const size_t numChans)
    {
        if (numChans < 2) throw Pothos::RangeException(
            "ChannelAligner::setNumChannels("+std::to_string(numChans)+")", "require numChans >= 2");

        for (size_t i = this->inputs().size(); i < numChans; i++)
        {
            this->setupInput(i, this->input(0)->dtype());
        }

        for (size_t i = this->outputs().size(); i < numChans; i++)
        {
            this->setupOutput(i, this->output(0)->dtype());
        }
    }

    long long dropped(void) const
    {
        return _droppedSamps;
    }

    void activate(void)
    {
        _nextTimeNs.resize(this->inputs().size(), 0);
    }

    void work(void);

    size_t sampsToTimeNs(const size_t samps)
    {
        return size_t(((samps*1e9)/_sampleRate) + 0.5);
    }

    size_t timeNsToSamps(const size_t timeNs)
    {
        return size_t(((timeNs*_sampleRate)/1e9) + 0.5);
    }

private:
    double _sampleRate;
    std::vector<long long> _nextTimeNs;
    long long _droppedSamps;
};

void ChannelAligner::work(void)
{
    if (this->workInfo().minInElements == 0) return;

    //search all inputs for relevant labels
    for (auto input : this->inputs())
    {
        for (const auto &label : input->labels())
        {
            if (label.index >= input->elements()) continue;

            //extract rx time and sample rate
            if (label.id == "rxRate")
            {
                _sampleRate = label.data.convert<double>();
            }
            else if (label.id == "rxTime")
            {
                const auto timeNs = label.data.convert<long long>();
                const size_t deltaSamps = label.index/input->buffer().dtype.size();
                const size_t deltaNs = this->sampsToTimeNs(deltaSamps);
                _nextTimeNs[input->index()] = timeNs - deltaNs;
            }
        }
    }

    //consume and dont forward inputs to force alignment
    size_t alignIndex = 0;
    auto alignTimeNs = _nextTimeNs[alignIndex++];
    size_t numElems = this->input(0)->buffer().elements();
    while (alignIndex < this->inputs().size())
    {
        auto input = this->input(alignIndex);
        const auto frontTimeNs = _nextTimeNs[alignIndex];
        numElems = std::min(numElems, input->buffer().elements());
        //if (input->buffer().elements()) std::cout << "  have " << input->buffer().elements() << " on " << alignIndex << std::endl;

        //front ticks are equal, check next channel
        if (frontTimeNs == alignTimeNs) alignIndex++;

        //front ticks are newer, reset ticks, start loop again
        else if (frontTimeNs > alignTimeNs)
        {
            alignTimeNs = frontTimeNs;
            alignIndex = 0;
        }

        //front ticks are older, consume and return
        else if (frontTimeNs < alignTimeNs)
        {
            const size_t deltaSamps = this->timeNsToSamps(alignTimeNs - frontTimeNs);
            const size_t consumeSamps = std::min(input->buffer().elements(), deltaSamps);
            const size_t consumeBytes = consumeSamps*input->buffer().dtype.size();
            //std::cout << "  catch up consume " << consumeSamps << " on " << alignIndex << std::endl;
            input->consume(consumeBytes);
            _nextTimeNs[input->index()] += this->sampsToTimeNs(consumeSamps);
            _droppedSamps += consumeSamps;
            return; //we get called again ASAP if inputs are available
        }
    }

    if (numElems == 0) return;
    //std::cout << "consume " << numElems << " on all " << std::endl;

    //we are in alignment, forward all outputs
    for (auto input : this->inputs())
    {
        auto buffer = input->buffer();
        buffer.length = numElems*buffer.dtype.size();
        this->output(input->index())->postBuffer(buffer);
        _nextTimeNs[input->index()] += this->sampsToTimeNs(numElems);
        input->consume(buffer.length);
    }
}

static Pothos::BlockRegistry registerChannelAligner(
    "/sdr/channel_aligner", &ChannelAligner::make);
