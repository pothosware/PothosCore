// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <cstdint>
#include <complex>
#include <cassert>
#include <iostream>

/***********************************************************************
 * |PothosDoc PreambleCorrelator
 *
 * The PreambleCorrelator searches an input unpacked bit stream from port 0
 * for a matching pattern and outputs the same bit stream with a tag
 * annotating the first bit of a match
 *
 * http://en.wikipedia.org/wiki/Hamming_distance
 *
 * |category /Digital
 * |keywords bit preamble correlate
 *
 * |param preamble The unpacked vector of bits representing preamble to match.
 * |default [1]
 *
 * |param thresh The threshold hamming distance for tagging.
 * |default 0
 *
 * |param label The label text for tagging first sample of correlator match.
 * |default "Matched!"
 * |widget StringEntry()
 *
 * |factory /blocks/preamble_correlator()
 * |setter setPreamble(preamble)
 * |setter setThreshold(thresh)
 * |setter setLabel(label)
 **********************************************************************/
class PreambleCorrelator : public Pothos::Block
{
public:
    static Block *make(void)
    {
        return new PreambleCorrelator();
    }

    PreambleCorrelator(void)
    {
        this->setupInput(0, typeid(unsigned char));
        this->setupOutput(0, typeid(unsigned char));
        //this->setupOutput(1, typeid(int));
        this->registerCall(this, POTHOS_FCN_TUPLE(PreambleCorrelator, setPreamble));
        this->registerCall(this, POTHOS_FCN_TUPLE(PreambleCorrelator, getPreamble));
        this->registerCall(this, POTHOS_FCN_TUPLE(PreambleCorrelator, setThreshold));
        this->registerCall(this, POTHOS_FCN_TUPLE(PreambleCorrelator, getThreshold));
        this->registerCall(this, POTHOS_FCN_TUPLE(PreambleCorrelator, setLabel));
        this->registerCall(this, POTHOS_FCN_TUPLE(PreambleCorrelator, getLabel));
        this->setPreamble(std::vector<unsigned char>(1, 1)); //initial update
        this->setThreshold(1); //initial update
        this->setLabel("Matched!"); //initial update
    }

    void setPreamble(const std::vector<unsigned char> preamble)
    {
        if (preamble.empty()) throw Pothos::InvalidArgumentException("PreambleCorrelator::setPreamble()", "taps cannot be empty");
        _preamble = preamble;
    }

    std::vector<unsigned char> getPreamble(void) const
    {
        return _preamble;
    }

    void setThreshold(int threshold)
    {
        _threshold = threshold;
    }

    int getThreshold()
    {
        return _threshold;
    }

    void setLabel(std::string label)
    {
        _label = label;
    }

    std::string getLabel()
    {
        return _label;
    }

    //! always use a circular buffer to avoid discontinuity over sliding window
    Pothos::BufferManager::Sptr getInputBufferManager(const std::string &, const std::string &)
    {
        return Pothos::BufferManager::make("circular");
    }

    void work(void)
    {
        auto inputPort = this->input(0);
        auto outputPort = this->output(0);
        //auto outputDistance = this->output(1);
        const auto &workInfo = this->workInfo();
        const int minElements = workInfo.minElements;

        if (minElements < (int) _preamble.size()) return;

        auto buffer = inputPort->buffer();
        if (buffer.length < (size_t) _preamble.size()) return;

        const auto N = minElements - _preamble.size();

        buffer.length = N;
        if (buffer.length != 0)
        {
            outputPort->postBuffer(buffer);
            inputPort->consume(N);
        }

        // Calculate Hamming distance
        auto in = inputPort->buffer().template as<unsigned char *>();
        //auto distance = outputDistance->buffer().template as<int *>();
        for (size_t n = 0; n < N; n++)
        {
            int dist = 0;

            // Count the number of bits set
            for (size_t i = 0; i < _preamble.size(); i++)
            {
                // A bit is set, so increment the distance
                dist += _preamble[i] ^ in[n+i];
            }
            // Emit a label if within the distance threshold
            if (dist <= _threshold)
            {
                outputPort->postLabel(Pothos::Label(_label, Pothos::Object(), n + _preamble.size()));
            }

            //distance[n] = dist;
        }
        //outputDistance->produce(N);
    }

private:
    int _threshold;
    std::string _label;
    std::vector<unsigned char> _preamble;
};

/***********************************************************************
 * registration
 **********************************************************************/
static Pothos::BlockRegistry registerPreambleCorrelator(
    "/blocks/preamble_correlator", &PreambleCorrelator::make);
