// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>

/***********************************************************************
 * |PothosDoc Infinite Source
 *
 * The infinite source is a test block for producing limitless output.
 * The primary use is for benchmarking the mechanics of the framework.
 *
 * This source will produce the entire output buffer and a null message
 * for every call to work. The only mechanism that throttles this block
 * is the fact that buffers and message tokenshave a limited resource pool,
 * that is only replenished by the consumption by downstream consumers.
 *
 * |category /Utility
 * |category /Sources
 * |keywords test
 *
 * |factory /blocks/infinite_source()
 **********************************************************************/
class InfiniteSource : public Pothos::Block
{
public:
    static Block *make(void)
    {
        return new InfiniteSource();
    }

    InfiniteSource(void)
    {
        this->setupOutput(0);
    }

    void work(void)
    {
        auto outputPort = this->output(0);
        outputPort->postMessage(Pothos::Object());
        outputPort->produce(outputPort->elements());
    }
};

static Pothos::BlockRegistry registerInfiniteSource(
    "/blocks/infinite_source", &InfiniteSource::make);

