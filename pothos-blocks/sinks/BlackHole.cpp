// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>

/***********************************************************************
 * |PothosDoc Black Hole
 *
 * The black hole block consumes all input provided on port 0.
 * This block is primarily used for testing purposes.
 *
 * |category /Sinks
 * |keywords sink null black
 *
 * |param dtype[Data Type] The datatype this block consumes.
 * |preview disable
 * |default "float32"
 *
 * |factory /blocks/sinks/black_hole(dtype)
 **********************************************************************/
class BlackHole : public Pothos::Block
{
public:
    static Block *make(const Pothos::DType &dtype)
    {
        return new BlackHole(dtype);
    }

    BlackHole(const Pothos::DType &dtype)
    {
        this->setupInput(0, dtype);
    }

    void work(void)
    {
        auto in0 = this->input(0);
        if (in0->hasMessage()) in0->popMessage();
        in0->consume(in0->elements());
    }
};

static Pothos::BlockRegistry registerBlackHole(
    "/blocks/sinks/black_hole", &BlackHole::make);
