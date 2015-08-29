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
 * with a label annotating the first symbol after the preamble match.
 *
 * This block supports operations on arbitrary symbol widths,
 * and therefore it may be used operationally on a bit-stream,
 * because a bit-stream is identically a symbol stream of N=1.
 *
 * http://en.wikipedia.org/wiki/Hamming_distance
 *
 * |category /Digital
 * |keywords bit symbol preamble correlate
 * |alias /blocks/preamble_correlator
 *
 * |param preamble A vector of symbols representing the preamble.
 * The width of each preamble symbol must the intended input stream.
 * |default [1]
 *
 * |param thresh[Threshold] The threshold hamming distance for preamble match detection.
 * |default 0
 *
 * |param frameStartId[Frame Start ID] The label ID that marks the first symbol of a correlator match.
 * |default "frameStart"
 * |widget StringEntry()
 *
 * |factory /comms/preamble_correlator()
 * |setter setPreamble(preamble)
 * |setter setThreshold(thresh)
 * |setter setFrameStartId(frameStartId)
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
        this->registerCall(this, POTHOS_FCN_TUPLE(PreambleCorrelator, setFrameStartId));
        this->registerCall(this, POTHOS_FCN_TUPLE(PreambleCorrelator, getFrameStartId));
        this->setPreamble(std::vector<unsigned char>(1, 1)); //initial update
        this->setThreshold(1); //initial update
        this->setFrameStartId("frameStart"); //initial update
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

    void setFrameStartId(std::string id)
    {
        _frameStartId = id;
    }

    std::string getFrameStartId(void) const
    {
        return _frameStartId;
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
                outputPort->postLabel(Pothos::Label(_frameStartId, Pothos::Object(), n + _preamble.size()));
            }

            //distance[n] = dist;
        }
        //outputDistance->produce(N);
    }

private:
    unsigned _threshold;
    std::string _frameStartId;
    std::vector<unsigned char> _preamble;
};

/***********************************************************************
 * registration
 **********************************************************************/
static Pothos::BlockRegistry registerPreambleCorrelator(
    "/comms/preamble_correlator", &PreambleCorrelator::make);

static Pothos::BlockRegistry registerPreambleCorrelatorOldPath(
    "/blocks/preamble_correlator", &PreambleCorrelator::make);
