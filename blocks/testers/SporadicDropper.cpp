// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <iostream>
#include <random>

/***********************************************************************
 * |PothosDoc Sporadic Dropper
 *
 * The sporadic dropper block passively forwards all data from input port 0
 * to output port 0 while randomly dropping entire buffers or messages.
 * This block is mainly used for recovery tolerance testing.
 *
 * |category /Testers
 * |category /Random
 * |keywords random drop
 *
 * |param probability The probability of an input being dropped.
 * A probability of 1 would mean every input, a probability of 0 would mean never.
 * |default 0.001
 *
 * |factory /blocks/sporadic_dropper()
 * |setter setProbability(probability)
 **********************************************************************/
class SporadicDropper : public Pothos::Block
{
public:
    static Block *make(void)
    {
        return new SporadicDropper();
    }

    SporadicDropper(void):
        _gen(_rd()),
        _probability(0.0)
    {
        this->setupInput(0);
        this->setupOutput(0, "", this->uid()); //unique domain because of buffer forwarding
        this->registerCall(this, POTHOS_FCN_TUPLE(SporadicDropper, setProbability));
        this->registerCall(this, POTHOS_FCN_TUPLE(SporadicDropper, getProbability));
    }

    void setProbability(const double prob)
    {
        if (prob > 1.0 or prob < 0.0) throw Pothos::RangeException(
            "SporadicDropper::setProbability("+std::to_string(prob)+")", "probability not in [0.0, 1.0]");
        _probability = prob;
    }

    double getProbability(void) const
    {
        return _probability;
    }

    void work(void)
    {
        auto inputPort = this->input(0);
        auto outputPort = this->output(0);

        //calculate if a drop will occur
        const bool drop = (std::generate_canonical<double, 10>(_gen) <= _probability);

        //randomly drop the input message if available
        if (inputPort->hasMessage())
        {
            auto m = inputPort->popMessage();
            if (not drop) outputPort->postMessage(m);
            return;
        }

        //get the input buffer
        const auto &buffer = inputPort->buffer();
        if (buffer.length == 0) return;
        inputPort->consume(inputPort->elements());

        //drop means do not post and remove all labels
        if (drop) for (const auto &label : inputPort->labels())
        {
            if (label.index >= inputPort->elements()) break;
            inputPort->removeLabel(label);
        }
        else outputPort->postBuffer(buffer);
    }

private:
    std::random_device _rd;
    std::mt19937 _gen;
    std::uniform_int_distribution<size_t> _randomId;

    double _probability;
};

static Pothos::BlockRegistry registerSporadicDropper(
    "/blocks/sporadic_dropper", &SporadicDropper::make);
