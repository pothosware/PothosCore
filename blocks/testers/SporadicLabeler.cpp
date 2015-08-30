// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <iostream>
#include <random>

/***********************************************************************
 * |PothosDoc Sporadic Labeler
 *
 * The sporadic labeler block passively forwards all data from input port 0
 * to output port 0 while randomly inserts labels at irregular intervals.
 * This block is mainly used for testing other blocks that deal with labels.
 *
 * |category /Testers
 * |category /Random
 * |category /Labels
 * |keywords random insert labels
 *
 * |param probability The probability of a label being associated with a random element.
 * A probability of 1 would mean every element, a probability of 0 would mean never.
 * |default 0.001
 *
 * |param ids[ID List] A list of label IDs that will be used.
 * When inserting a label, an ID will be randomly selected from this list.
 * |default ["test"]
 *
 * |factory /blocks/sporadic_labeler()
 * |setter setProbability(probability)
 * |setter setIdList(ids)
 **********************************************************************/
class SporadicLabeler : public Pothos::Block
{
public:
    static Block *make(void)
    {
        return new SporadicLabeler();
    }

    SporadicLabeler(void):
        _gen(_rd()),
        _probability(0.0)
    {
        this->setupInput(0);
        this->setupOutput(0, "", this->uid()); //unique domain because of buffer forwarding
        this->registerCall(this, POTHOS_FCN_TUPLE(SporadicLabeler, setProbability));
        this->registerCall(this, POTHOS_FCN_TUPLE(SporadicLabeler, getProbability));
        this->registerCall(this, POTHOS_FCN_TUPLE(SporadicLabeler, setIdList));
        this->registerCall(this, POTHOS_FCN_TUPLE(SporadicLabeler, getIdList));
    }

    void setProbability(const double prob)
    {
        if (prob > 1.0 or prob < 0.0) throw Pothos::RangeException(
            "SporadicLabeler::setProbability("+std::to_string(prob)+")", "probability not in [0.0, 1.0]");
        _probability = prob;
    }

    double getProbability(void) const
    {
        return _probability;
    }

    void setIdList(const std::vector<std::string> &ids)
    {
        _ids = ids;
        if (_ids.empty()) return;
        _randomId = std::uniform_int_distribution<size_t>(0, _ids.size()-1);
    }

    std::vector<std::string> getIdList(void) const
    {
        return _ids;
    }

    void work(void)
    {
        auto inputPort = this->input(0);
        auto outputPort = this->output(0);

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

            for (size_t i = 0; i < inputPort->elements(); i++)
            {
                if (std::generate_canonical<double, 10>(_gen) <= _probability)
                {
                    Pothos::Label label;
                    label.index = i;
                    label.width = buffer.dtype.size();
                    if (not _ids.empty()) label.id = _ids.at(_randomId(_gen));
                    outputPort->postLabel(label);
                }
            }
        }
    }

private:
    std::random_device _rd;
    std::mt19937 _gen;
    std::uniform_int_distribution<size_t> _randomId;

    double _probability;
    std::vector<std::string> _ids;
};

static Pothos::BlockRegistry registerSporadicLabeler(
    "/blocks/sporadic_labeler", &SporadicLabeler::make);
