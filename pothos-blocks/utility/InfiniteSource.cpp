// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <iostream>
#include <algorithm> //min/max

/***********************************************************************
 * |PothosDoc Infinite Source
 *
 * The infinite source is a test block for producing limitless output.
 * The primary use is for benchmarking the mechanics of the framework.
 * The configuration options determine what types of resources are
 * produced in the work() function of the infinite source block.
 *
 * Although named infinite, the actual mechanism that throttles this block
 * is the fact that buffers and message tokens have a limited resource pool
 * that is only replenished by the consumption by downstream consumers.
 *
 * |category /Utility
 * |category /Sources
 * |keywords test
 *
 * |param enableBuffers[Enable Buffers] True to enable production of buffers.
 * When enabled, one buffer will be produced per call to work().
 * The buffer's memory will not be filled or cleared in any way.
 * |default false
 * |option [Enable] true
 * |option [Disable] false
 * |preview valid
 *
 * |param bufferMTU[Buffer MTU] The maximum size of a buffer produced per work().
 * A MTU of 0 bytes means that the entire buffer will be produced each work().
 * Buffer production must be enabled for this option to have an effect.
 * |units bytes
 * |default 0
 * |preview valid
 *
 * |param enableLabels[Enable Labels] True to enable production of labels.
 * When enabled, one label will be produced per call to work().
 * Buffer production must be enabled for this option to have an effect.
 * |default false
 * |option [Enable] true
 * |option [Disable] false
 * |preview valid
 *
 * |param enableMessages[Enable Messages] True to enable production of messages.
 * When enabled, one null message will be produced per call to work().
 * |default false
 * |option [Enable] true
 * |option [Disable] false
 * |preview valid
 *
 * |factory /blocks/infinite_source()
 * |setter setBufferMTU(bufferMTU)
 * |setter enableBuffers(enableBuffers)
 * |setter enableLabels(enableLabels)
 * |setter enableMessages(enableMessages)
 **********************************************************************/
class InfiniteSource : public Pothos::Block
{
public:
    static Block *make(void)
    {
        return new InfiniteSource();
    }

    InfiniteSource(void):
        _bufferMTU(0),
        _enableBuffers(false),
        _enableLabels(false),
        _enableMessages(false)
    {
        this->setupOutput(0);
        this->registerCall(this, POTHOS_FCN_TUPLE(InfiniteSource, setBufferMTU));
        this->registerCall(this, POTHOS_FCN_TUPLE(InfiniteSource, enableBuffers));
        this->registerCall(this, POTHOS_FCN_TUPLE(InfiniteSource, enableLabels));
        this->registerCall(this, POTHOS_FCN_TUPLE(InfiniteSource, enableMessages));
    }

    void setBufferMTU(const size_t mtu)
    {
        _bufferMTU = mtu;
    }

    void enableBuffers(const bool enb)
    {
        _enableBuffers = enb;
    }

    void enableLabels(const bool enb)
    {
        _enableLabels = enb;
    }

    void enableMessages(const bool enb)
    {
        _enableMessages = enb;
    }

    void deactivate(void)
    {
        //print a short summary of the production
        auto outputPort = this->output(0);
        if (_enableBuffers) std::cout << this->getName() << " total bytes " << outputPort->totalElements() << std::endl;
        if (_enableBuffers) std::cout << this->getName() << " total buffers " << outputPort->totalBuffers() << std::endl;
        if (_enableLabels) std::cout << this->getName() << " total labels " << outputPort->totalLabels() << std::endl;
        if (_enableMessages) std::cout << this->getName() << " total messages " << outputPort->totalMessages() << std::endl;
    }

    void work(void)
    {
        auto outputPort = this->output(0);

        //message production
        if (_enableMessages)
        {
            outputPort->postMessage(Pothos::Object());
        }

        //buffer production
        if (_enableBuffers)
        {
            size_t bytes = outputPort->elements();
            if (_bufferMTU != 0) bytes = std::min(bytes, _bufferMTU);
            outputPort->produce(bytes);

            //label production
            if (_enableLabels)
            {
                Pothos::Label label;
                label.index = 0;
                outputPort->postLabel(label);
            }
        }
    }

private:
    size_t _bufferMTU;
    bool _enableBuffers;
    bool _enableLabels;
    bool _enableMessages;
};

static Pothos::BlockRegistry registerInfiniteSource(
    "/blocks/infinite_source", &InfiniteSource::make);

