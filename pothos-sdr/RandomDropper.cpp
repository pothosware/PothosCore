// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>

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
 * |factory /sdr/random_dropper()
 **********************************************************************/
class RandomDropper : public Pothos::Block
{
public:
    static Block *make(void)
    {
        return new RandomDropper();
    }

    RandomDropper(void)
    {
        this->setupInput(0);
        this->setupOutput(0, "", this->uid()); //unique domain because of buffer forwarding
    }

    void work(void);

private:
};

void RandomDropper::work(void)
{
    auto inputPort = this->input(0);
    auto outputPort = this->output(0);

    inputPort->consume(inputPort->elements());
    outputPort->postBuffer(inputPort->buffer());
}

static Pothos::BlockRegistry registerRandomDropper(
    "/sdr/random_dropper", &RandomDropper::make);
