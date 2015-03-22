// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <random>
#include <iostream>

/***********************************************************************
 * |PothosDoc Random Dropper
 *
 * The random dropper block forwards an input stream to an output stream.
 * There is a random chance that a chunk of samples will be dropped.
 * A new timestamp will be inserted after the drop to re-establish sync.
 * Use this block to simulate overflows from an SDR source to test recovery.
 *
 * |category /SDR
 * |keywords drop overflow channel
 *
 * |param dropSize[Drop size] The number of samples to randomly drop.
 * |units samples
 * |default 1024
 *
 * |param probability The probability that a drop event will occur.
 * A probability of 1 would mean a drop for every work() event,
 * a probability of 0 would mean that drops never occur.
 * |default 0.001
 *
 * |factory /sdr/random_dropper()
 * |setter setDropSize(dropSize)
 * |setter setProbability(probability)
 **********************************************************************/
class RandomDropper : public Pothos::Block
{
public:
    static Block *make(void)
    {
        return new RandomDropper();
    }

    RandomDropper(void):
        _sampleRate(1.0),
        _nextTimeNs(0),
        _postTime(false),
        _dropSize(1024),
        _numLeftToDrop(0),
        _gen(_rd()),
        _probability(0.0)
    {
        this->setupInput(0);
        this->setupOutput(0, "", this->uid()); //unique domain because of buffer forwarding
        this->registerCall(this, POTHOS_FCN_TUPLE(RandomDropper, setDropSize));
        this->registerCall(this, POTHOS_FCN_TUPLE(RandomDropper, setProbability));
        this->registerCall(this, POTHOS_FCN_TUPLE(RandomDropper, getProbability));
    }

    void setDropSize(const size_t num)
    {
        _dropSize = num;
    }

    void setProbability(const double prob)
    {
        if (prob > 1.0 or prob < 0.0) throw Pothos::RangeException(
            "RandomDropper::setProbability("+std::to_string(prob)+")", "probability not in [0.0, 1.0]");
        _probability = prob;
    }

    double getProbability(void) const
    {
        return _probability;
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
    long long _nextTimeNs;
    bool _postTime;
    size_t _dropSize;
    size_t _numLeftToDrop;
    std::random_device _rd;
    std::mt19937 _gen;
    double _probability;
};

void RandomDropper::work(void)
{
    auto inputPort = this->input(0);
    auto outputPort = this->output(0);
    if (inputPort->elements() == 0) return;

    //search all inputs for relevant labels
    for (const auto &label : inputPort->labels())
    {
        if (label.index >= inputPort->elements()) continue;

        //extract rx time and sample rate
        if (label.id == "rxRate")
        {
            _sampleRate = label.data.convert<double>();
        }
        else if (label.id == "rxTime")
        {
            const auto timeNs = label.data.convert<long long>();
            const size_t deltaSamps = label.index/inputPort->buffer().dtype.size();
            const size_t deltaNs = this->sampsToTimeNs(deltaSamps);
            _nextTimeNs = timeNs - deltaNs;
        }
    }

    //need to drop?
    if (_numLeftToDrop != 0)
    {
        const size_t numElems = std::min(_numLeftToDrop, inputPort->buffer().elements());
        const size_t numBytes = numElems*inputPort->buffer().dtype.size();
        inputPort->consume(numBytes);
        _nextTimeNs += sampsToTimeNs(numElems);
        _numLeftToDrop -= numElems;
        return;
    }

    //cause a drop?
    if (std::generate_canonical<double, 10>(_gen) <= _probability)
    {
        _postTime = true;
        _numLeftToDrop = _dropSize;
        std::cerr << "D" << std::flush;
        return this->yield();
    }

    //post time when instructed
    if (_postTime)
    {
        const Pothos::Label label("rxTime", _nextTimeNs, 0);
        outputPort->postLabel(label);
        _postTime = false;
    }

    //normal forward activity
    inputPort->consume(inputPort->elements());
    outputPort->postBuffer(inputPort->buffer());
    _nextTimeNs += sampsToTimeNs(inputPort->buffer().elements());
}

static Pothos::BlockRegistry registerRandomDropper(
    "/sdr/random_dropper", &RandomDropper::make);
