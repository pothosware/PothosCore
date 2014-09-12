// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <cstring> //memset

/***********************************************************************
 * |PothosDoc Delay
 *
 * The delay block imposes a constant delay in stream elements.
 * The implementation passively forwards inputs to outputs,
 * without incuring any memory copying overhead.
 *
 * |category /Utility
 * |keywords delay time
 *
 * |param dtype[Data Type] The datatype this block consumes.
 * |preview disable
 * |default "float32"
 * |widget StringEntry()
 *
 * |param delay The delay in number of stream elements.
 * |default 0
 *
 * |factory /blocks/delay(dtype)
 * |setter setDelay(delay)
 **********************************************************************/
class Delay : public Pothos::Block
{
public:
    static Block *make(const Pothos::DType &dtype)
    {
        return new Delay(dtype);
    }

    Delay(const Pothos::DType &dtype):
        _deltaElements(0)
    {
        this->setupInput(0, dtype);
        this->setupOutput(0, dtype);
        this->registerCall(this, POTHOS_FCN_TUPLE(Delay, setDelay));
        this->registerCall(this, POTHOS_FCN_TUPLE(Delay, getDelay));
    }

    void setDelay(const int elements)
    {
        _deltaElements = elements;
    }

    int getDelay(void) const
    {
        return _deltaElements;
    }

    void work(void)
    {
        auto in0 = this->input(0);
        auto out0 = this->output(0);
        const long long delta = (long long)(in0->totalElements()) - (long long)(out0->totalElements()) + _deltaElements;

        //consume but not produce (drops elements)
        if (delta < 0)
        {
            in0->consume(std::min(in0->elements(), size_t(-delta)));
            return;
        }

        //produce but not consume (inserts zeros)
        if (delta > 0)
        {
            const size_t numElems = std::min(out0->elements(), size_t(delta));
            std::memset(out0->buffer().as<void *>(), 0, numElems*out0->dtype().size());
            out0->produce(numElems);
            return;
        }

        //otherwise just forward the buffer
        auto buffer = in0->buffer();
        if (buffer.length > 0)
        {
            out0->postBuffer(buffer);
            in0->consume(in0->elements());
        }
    }

private:
    int _deltaElements;
};

static Pothos::BlockRegistry registerDelay(
    "/blocks/delay", &Delay::make);
