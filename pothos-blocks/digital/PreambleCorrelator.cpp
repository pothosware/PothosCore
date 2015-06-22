// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <cstdint>
#include <complex>
#include <cassert>
#include <iostream>

//provide __popcnt()
#ifdef _MSC_VER
#  include <intrin.h>
#elif __GNUC__
#  define __popcnt __builtin_popcount
#else
#  error "provide __popcnt() for this compiler"
#endif

/***********************************************************************
 * |PothosDoc Preamble Correlator
 *
 * The Preamble Correlator searches an input symbol stream on port 0
 * for a matching pattern and forwards the stream to output port 0
 * with a label annotating the first bit after the preamble match.
 *
 * http://en.wikipedia.org/wiki/Hamming_distance
 *
 * |category /Digital
 * |keywords bit preamble correlate
 *
 * |param preamble The unpacked vector of bits representing preamble to match.
 * |default [1]
 *
 * |param thresh The threshold hamming distance for preamble match detection.
 * |default 0
 *
 * |param label The label id that marks the first sample of a correlator match.
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

    PreambleCorrelator(void):
        _threshold(0)
    {
        this->setupInput(0, typeid(unsigned char));
        this->setupOutput(0, typeid(unsigned char), this->uid()); //unique domain because of buffer forwarding
        //this->setupOutput(1, typeid(unsigned));
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
        if (preamble.empty()) throw Pothos::InvalidArgumentException("PreambleCorrelator::setPreamble()", "preamble cannot be empty");
        _preamble = preamble;
    }

    std::vector<unsigned char> getPreamble(void) const
    {
        return _preamble;
    }

    void setThreshold(const unsigned threshold)
    {
        _threshold = threshold;
    }

    unsigned getThreshold(void) const
    {
        return _threshold;
    }

    void setLabel(std::string label)
    {
        _label = label;
    }

    std::string getLabel(void) const
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

        //require preamble size + 1 elements to perform processing
        inputPort->setReserve(_preamble.size()+1);
        auto buffer = inputPort->buffer();
        if (buffer.length <= (size_t) _preamble.size()) return;

        //due to search window, the last preamble size elements are used
        //consume and forward all processable elements of the input buffer
        buffer.length -= _preamble.size();
        outputPort->postBuffer(buffer);
        inputPort->consume(buffer.length);

        // Calculate Hamming distance at each position looking for match
        // When a match is found a label is created after the preamble
        auto in = buffer.as<unsigned char *>();
        //auto distance = outputDistance->buffer().template as<unsigned *>();
        for (size_t n = 0; n < buffer.length; n++)
        {
            unsigned dist = 0;

            // Count the number of bits set
            for (size_t i = 0; i < _preamble.size(); i++)
            {
                // A bit is set, so increment the distance
                dist += __popcnt(_preamble[i] ^ in[n+i]);
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
    unsigned _threshold;
    std::string _label;
    std::vector<unsigned char> _preamble;
};

/***********************************************************************
 * registration
 **********************************************************************/
static Pothos::BlockRegistry registerPreambleCorrelator(
    "/blocks/preamble_correlator", &PreambleCorrelator::make);
